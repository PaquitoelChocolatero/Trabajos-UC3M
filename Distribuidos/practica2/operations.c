#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include "operations.h"

sqlite3 *registered_db, *active_db;		//Base de datos de usuarios registrados y usuarios activos
int registered_rc, active_rc;
char *err = 0;				//Variable de control
char concat_sql_op[1000];
char *sql_op;
int exists = 0;
char selected_items[42][1000];   // 42/3 (campos por fichero) = 14 archivos por usuario
char *userIP; //User extracted from ip



static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   char user[4] = "USER";
   for(i = 0; i<argc; i++) {
        if(strcmp(azColName[i],user)!=0){   //Solo nos interesan las columnas que no son 'USER'
            sprintf(selected_items[i], "%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
        }
   }
   printf("\n");
   return 0;
}

int elementExists(void *NotUsed, int argc, char **argv, char **azColName) {
    if(argc>0){
        exists = 1;
        printf("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");
    }
   else exists = 0;
   return 0;
}

int ip2user(void *NotUsed, int argc, char **argv, char **azColName) {
    if(argc>0){
        exists = 1;
        size_t length = strlen(argv[0])+1;
        userIP = malloc(length);
        memcpy(userIP, argv[0], length);
    }
    else exists = 0;
    return 0;
}

void checkError(){
    if( registered_rc != SQLITE_OK || active_rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
    }
}



/*
*   GENERATE DATABASES
*/
void startServer()
{
    //Abrir la base de datos de registrados
    registered_rc = sqlite3_open("registered.db", &registered_db);

	if(registered_rc)fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(registered_db));

    //Comprobar si la tabla usuarios existe
    sql_op = "SELECT * FROM sqlite_master WHERE type='table' AND name='USERS';";
    registered_rc = sqlite3_exec(registered_db, sql_op, elementExists, 0, &err);
    checkError();   //Comprobar errores

    //Si la tabla de usuarios no existe la creamos
    if(exists == 0){
        //Creamos la tabla de usuarios
        sql_op = "CREATE TABLE USERS("                              \
                 "USER           TEXT   PRIMARY KEY     NOT NULL);";

        registered_rc = sqlite3_exec(registered_db, sql_op, callback, 0, &err);
        checkError();

        //Creamos la tabla de ficheros
        sql_op = "CREATE TABLE FILES("                              \
                 "USER           TEXT   NOT NULL,"  \
                 "NAME           TEXT   NOT NULL,"  \
                 "DESCRIPTION    TEXT   NOT NULL,"  \
                 "PRIMARY KEY(USER, NAME),"         \
                 "FOREIGN KEY(USER) REFERENCES USERS(USER) ON DELETE CASCADE);";

        registered_rc = sqlite3_exec(registered_db, sql_op, callback, 0, &err);
        checkError();
    }
    sqlite3_close(registered_db); //Cerramos la base de datos

    //Abrir la base de datos de activos
    active_rc = sqlite3_open("active.db", &active_db);
	if(active_rc)fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(active_db));

    //Comprobar si la tabla usuarios existe
    sql_op = "SELECT * FROM sqlite_master WHERE type='table' AND name='USERS';";
    active_rc = sqlite3_exec(active_db, sql_op, elementExists, 0, &err);
    checkError();

    //Si la tabla de usuarios no existe la creamos
    if(exists == 0){
        sql_op = "CREATE TABLE USERS("                              \
                 "USER           TEXT   PRIMARY KEY     NOT NULL,"  \
                 "IP             TEXT                   NOT NULL,"  \
                 "PORT           INT                    NOT NULL);";

        active_rc = sqlite3_exec(active_db, sql_op, callback, 0, &err);
        checkError();

        sql_op = "CREATE TABLE FILES("                              \
                 "USER           TEXT   NOT NULL,"  \
                 "NAME           TEXT   NOT NULL,"  \
                 "DESCRIPTION    TEXT   NOT NULL,"  \
                 "PRIMARY KEY(USER, NAME),"         \
                 "FOREIGN KEY(USER) REFERENCES USERS(USER) ON DELETE CASCADE);";

        active_rc = sqlite3_exec(active_db, sql_op, callback, 0, &err);
        checkError();
    }
    sqlite3_close(active_db);
}



/*
*   REGISTER USER
*/
int registerUser(char *user)
{
    registered_rc = sqlite3_open("registered.db", &registered_db);
	if(registered_rc) fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(registered_db));

    //Comprobar si el usuario existe
    strcpy(concat_sql_op, "SELECT * FROM USERS WHERE user='");
    strcat(concat_sql_op, user);
    strcat(concat_sql_op, "';");
    registered_rc = sqlite3_exec(registered_db, concat_sql_op, elementExists, 0, &err);
    checkError();

    //Si el usuario no existe le podemos registrar
    if(exists == 0){
        //Concatenamos la instrucción con el usuario deseado
        strcpy(concat_sql_op, "INSERT INTO USERS VALUES('");
        strcat(concat_sql_op, user);
        strcat(concat_sql_op, "');");
        registered_rc = sqlite3_exec(registered_db, concat_sql_op, callback, 0, &err);
        checkError();
    }else{
        printf("USERNAME IN USE\n");
        return -1;
    }
    printf("REGISTER OK\n");
    sqlite3_close(registered_db); //Cerramos la base de datos
    return 1;
}



/*
*   UNREGISTER USER
*/
int unregisterUser(char *user)
{
    //Abrir la base de datos de activos
    active_rc = sqlite3_open("active.db", &active_db);
	if(active_rc)fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(active_db));

    //Linkeamos la base de usuarios activos a la de registrados
    sql_op = "ATTACH 'registered.db' AS registered;";
    active_rc = sqlite3_exec(active_db, sql_op, callback, 0, &err);
    checkError();

    //Comprobar si el usuario existe
    strcpy(concat_sql_op, "SELECT * FROM registered.USERS WHERE user='");
    strcat(concat_sql_op, user);
    strcat(concat_sql_op, "';");
    active_rc = sqlite3_exec(active_db, concat_sql_op, elementExists, 0, &err);
    checkError();

    //Si el usuario existe
    if(exists == 1){
        //Comprobar si el usuario está conectado
        strcpy(concat_sql_op, "SELECT * FROM USERS WHERE user='");
        strcat(concat_sql_op, user);
        strcat(concat_sql_op, "';");
        active_rc = sqlite3_exec(active_db, concat_sql_op, elementExists, 0, &err);
        checkError();

        //Si el usuario está conectado
        if(exists == 1){

            // //Borramos al usuario de active
            // strcpy(concat_sql_op, "DELETE FROM USERS WHERE user='");
            // strcat(concat_sql_op, userIP);
            // strcat(concat_sql_op, "';");
            // active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
            // checkError();

            //Borramos al usuario de registered
            strcpy(concat_sql_op, "DELETE FROM registered.USERS WHERE user='");
            strcat(concat_sql_op, userIP);
            strcat(concat_sql_op, "';");
            active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
            checkError();
        }else{
            printf("USER IS NOT CONNECTED\n");
            return -1;
        }
    }else{
        printf("USER DOES NOT EXIST\n");
        return -1;
    }
    printf("UNREGISTER OK\n");
	sqlite3_close(active_db);
    return 1;
}



/*
*   CONNECT USER IP PORT
*/
int connectUser(char *user, char *ip, int port)
{
    //Abrir la base de datos de registrados
    registered_rc = sqlite3_open("registered.db", &registered_db);
	if(registered_rc) fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(registered_db));

    //Linkeamos la base de usuarios activos a la de registrados
    sql_op = "ATTACH 'active.db' AS active;";
    registered_rc = sqlite3_exec(registered_db, sql_op, callback, 0, &err);
    checkError();

    //Comprobar si el usuario existe
    strcpy(concat_sql_op, "SELECT * FROM USERS WHERE user='");
    strcat(concat_sql_op, user);
    strcat(concat_sql_op, "';");
    registered_rc = sqlite3_exec(registered_db, concat_sql_op, elementExists, 0, &err);
    checkError();

    //Si el usuario existe le podemos conectar
    if(exists == 1){
        
        //Comprobar si el usuario está conectado
        strcpy(concat_sql_op, "SELECT * FROM active.USERS WHERE user='");
        strcat(concat_sql_op, user);
        strcat(concat_sql_op, "';");
        registered_rc = sqlite3_exec(registered_db, concat_sql_op, elementExists, 0, &err);
        checkError();
        printf("%i\n", exists);

        //Si el usuario no está conectado
        if(exists == 0){
            char str_port[3];
            sprintf(str_port,"%d",port);
            //Concatenamos la instrucción con el usuario deseado
            strcpy(concat_sql_op, "INSERT INTO active.USERS (USER,IP,PORT) VALUES('");
            strcat(concat_sql_op, user);
            strcat(concat_sql_op, "','");
            strcat(concat_sql_op, ip);
            strcat(concat_sql_op, "',");
            strcat(concat_sql_op, str_port);
            strcat(concat_sql_op, ");");
            registered_rc = sqlite3_exec(registered_db, concat_sql_op, callback, 0, &err);
            checkError();

            //Volcamos los datos del usuario en la tabla FICHEROS de los usuarios activos
            strcpy(concat_sql_op, "INSERT INTO active.FILES SELECT * FROM main.FILES WHERE user='");
            strcat(concat_sql_op, user);
            strcat(concat_sql_op, "';");
            registered_rc = sqlite3_exec(registered_db, concat_sql_op, callback, 0, &err);
            checkError();

            //Borramos los datos de active
            strcpy(concat_sql_op, "DELETE FROM FILES WHERE user='");
            strcat(concat_sql_op, user);
            strcat(concat_sql_op, "';");
            registered_rc = sqlite3_exec(registered_db, concat_sql_op, callback, 0, &err);
            checkError();
        }else{
            printf("USER ALREADY CONNECTED\n");
            return 2;
        }
    }else{
        printf("CONNECT FAIL, USER DOES NOT EXIST\n");
        return 1;
    }
    printf("CONNECT OK\n");
    sqlite3_close(registered_db);
    return 0;
}



/*
*   DISCONNECT
*/
int disconnectUser(char *ip)
{
    //Abrir la base de datos de activos
    active_rc = sqlite3_open("active.db", &active_db);
	if(active_rc)fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(active_db));

    //Comprobar si el usuario está conectado
    strcpy(concat_sql_op, "SELECT * FROM USERS WHERE ip='");
    strcat(concat_sql_op, ip);
    strcat(concat_sql_op, "';");
    active_rc = sqlite3_exec(active_db, concat_sql_op, ip2user, 0, &err);
    checkError();

    //Si el usuario está conectado
    if(exists == 1){

        //Linkeamos la base de usuarios activos a la de registrados
        sql_op = "ATTACH 'registered.db' AS registered;";
        active_rc = sqlite3_exec(active_db, sql_op, callback, 0, &err);
        checkError();

        //Volcamos los datos del usuario en la tabla FICHEROS de los usuarios activos
        strcpy(concat_sql_op, "INSERT INTO registered.FILES SELECT * FROM main.FILES WHERE user='");
        strcat(concat_sql_op, userIP);
        strcat(concat_sql_op, "';");
        active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
        checkError();

        //Borramos al usuario de active
        strcpy(concat_sql_op, "DELETE FROM USERS WHERE user='");
        strcat(concat_sql_op, userIP);
        strcat(concat_sql_op, "';");
        active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
        checkError();
    }else{
        printf("DISCONNECT FAIL / USER NOT CONNECTED\n");
        return -1;
    }
    printf("DISCONNECT OK\n");
	sqlite3_close(active_db);
    return 1;
}



/*
*   PUBLISH FILE DESCRIPTION
*/
int publishFile(char *ip, char *file, char *description)
{
    //Abrir la base de datos de activos
    active_rc = sqlite3_open("active.db", &active_db);
	if(active_rc) fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(active_db));

    //Comprobar si el usuario está conectado
    strcpy(concat_sql_op, "SELECT * FROM USERS WHERE ip='");
    strcat(concat_sql_op, ip);
    strcat(concat_sql_op, "';");
    active_rc = sqlite3_exec(active_db, concat_sql_op, ip2user, 0, &err);
    checkError();

    //Si el usuario está conectado
    if(exists == 1){

        //Comprobar si el archivo ya existe
        strcpy(concat_sql_op, "SELECT * FROM FILES WHERE user='");
        strcat(concat_sql_op, userIP);
        strcat(concat_sql_op, "' AND name=';");
        strcat(concat_sql_op, file);
        strcat(concat_sql_op, "';");
        active_rc = sqlite3_exec(active_db, concat_sql_op, elementExists, 0, &err);
        checkError();
        
        //Si el usuario está conectado
        if(exists == 0){
            //Insertamos el archivo
            strcpy(concat_sql_op, "INSERT INTO FILES (USER,NAME,DESCRIPTION) VALUES('");
            strcat(concat_sql_op, userIP);
            strcat(concat_sql_op, "','");
            strcat(concat_sql_op, file);
            strcat(concat_sql_op, "','");
            strcat(concat_sql_op, description);
            strcat(concat_sql_op, "');");
            active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
            checkError();
        }else{
            printf("PUBLISH FAIL, CONTENT ALREADY PUBLISHED\n");
            return 3;
        }
    }else{
        printf("PUBLISH FAIL, USER NOT CONNECTED\n");
        return 2;
    }
    printf("PUBLISH OK\n");
	sqlite3_close(active_db);
    return 0;
}



/*
*   DELETE FILE
*/
int deleteFile(char *ip, char *file)
{
    //Abrir la base de datos de activos
    active_rc = sqlite3_open("active.db", &active_db);
	if(active_rc)fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(active_db));

    //Comprobar si el usuario está conectado
    strcpy(concat_sql_op, "SELECT * FROM USERS WHERE ip='");
    strcat(concat_sql_op, ip);
    strcat(concat_sql_op, "';");
    active_rc = sqlite3_exec(active_db, concat_sql_op, ip2user, 0, &err);
    checkError();

    //Si el usuario está conectado
    if(exists == 1){
        //Comprobar si el archivo existe
        strcpy(concat_sql_op, "SELECT * FROM FILES WHERE user='");
        strcat(concat_sql_op, userIP);
        strcat(concat_sql_op, "' AND name='");
        strcat(concat_sql_op, file);
        strcat(concat_sql_op, "';");
        active_rc = sqlite3_exec(active_db, concat_sql_op, ip2user, 0, &err);
        checkError();

        if(exists == 1){
            //Borramos el archivo
            strcpy(concat_sql_op, "DELETE FROM FILES WHERE user='");
            strcat(concat_sql_op, userIP);
            strcat(concat_sql_op, "' AND name='");
            strcat(concat_sql_op, file);
            strcat(concat_sql_op, "';");
            active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
            checkError();
        }else{
            printf("DELETE FAIL, CONTENT NOT PUBLISHED\n");
            return 3;
        }
    }else{
        printf("DELETE FAIL, USER  NOT CONNECTED\n");
        return 2;
    }
    printf("DELETE OK\n");
	sqlite3_close(active_db);
    return 0;
}



/*
*   LIST_USERS
*/
int list_users()
{
    //Abrir la base de datos de activos
    active_rc = sqlite3_open("active.db", &active_db);
	if(active_rc)fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(active_db));

    //Listar usuarios
    strcpy(concat_sql_op, "SELECT * FROM USERS;");
    active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
    checkError();

    printf("LIST_USERS OK");
	sqlite3_close(active_db);
    return 0;
}



/*
*   LIST_CONTENT USER
*/
int list_content(char *user)
{
    //Abrir la base de datos de activos
    active_rc = sqlite3_open("active.db", &active_db);
	if(active_rc)fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(active_db));

    //Linkeamos la base de usuarios activos a la de registrados
    sql_op = "ATTACH 'registered.db' AS registered;";
    active_rc = sqlite3_exec(active_db, sql_op, callback, 0, &err);
    checkError();

    //Comprobar si el usuario existe
    strcpy(concat_sql_op, "SELECT * FROM registered.USERS WHERE user='");
    strcat(concat_sql_op, user);
    strcat(concat_sql_op, "';");
    active_rc = sqlite3_exec(active_db, concat_sql_op, elementExists, 0, &err);
    checkError();

    //Si el usuario existe
    if(exists == 1){

        //Comprobar si el usuario está conectado
        strcpy(concat_sql_op, "SELECT * FROM USERS WHERE user='");
        strcat(concat_sql_op, user);
        strcat(concat_sql_op, "';");
        active_rc = sqlite3_exec(active_db, concat_sql_op, elementExists, 0, &err);
        checkError();

        //Si el usuario está conectado
        if(exists == 1){
            //Listar archivos del usuario
            strcpy(concat_sql_op, "SELECT * FROM FILES WHERE user='");
            strcat(concat_sql_op, user);
            strcat(concat_sql_op, "';");
            active_rc = sqlite3_exec(active_db, concat_sql_op, callback, 0, &err);
            checkError();
        }else{
            printf("LIST_CONTENT FAIL, USER NOT CONNECTED\n");
            return 2;
        }
    }else{
        printf("LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST\n");
        return 3;
    }
    printf("LIST_CONTENT OK");
	sqlite3_close(active_db);
    return 0;
}