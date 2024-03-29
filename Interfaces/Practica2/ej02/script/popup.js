$(document).ready(function(){
    var modal = document.querySelector(".modal");
    var confirmation = document.querySelector(".confirmation");
    var addition = document.querySelector(".addition");
    var share = document.querySelector(".share");

    //Set cookie values
    if(localStorage.getItem('user') != null){
        $('.dropbtn-user a.active').html("<img src='images/user.png' id='userImg'></img>" + localStorage.getItem('user'));
        $('p#Welcome').text('Welcome ' + localStorage.getItem('user') + '!');
    }
    
    //Todo el contenido en un array para que sea dinámico y poner añadir/borrar
    var cards = [  
        [
            "Tasks to Do",
            [
                [
                    1,
                    "images/office.jpg",
                    "Daily standup meeting",
                    "26-09-2019"
                ],
                [
                    2,
                    "images/hire.jpg",
                    "Hire new UI/UX developer",
                    "10-12-2019"
                ]
            ]
        ],
        [
            "In progress",
            [
                [
                    1,
                    "images/fsf.jpg",
                    "Send FSF new proposal",
                    "26-11-2019"
                ],
            ]
        ],
        [
            "Finished",
            [
                [
                    1,
                    "images/office.jpg",
                    "Meeting with CEO",
                    "25-09-2019"
                ],
                [
                    2,
                    "images/brainstorm.png",
                    "Brainstorming for new projects",
                    "21-09-2019"
                ],
                [
                    3,
                    "images/coffe.jpg",
                    "Buy new coffe machine",
                    "15-09-2019"
                ],
                [
                    4,
                    "images/office.jpg",
                    "Project review",
                    "10-09-2019"
                ]
            ]
        ]
    ];

    //Función para pintar todas las tarjetitas
    $(function(){
        var it = 1;//Añadimos 1 a todo para obviar la columna auxiliar
        for(i=0; i<cards.length; i++){
            //Creamos una columna
            $('.column:first').clone(true, true).appendTo('.row');
            //Le ponemos el título correspondiente
            document.getElementsByClassName("card-title")[i+1].innerHTML = cards[i][0];
            for(j=0; j<cards[i][1].length; j++){
                //La primera ya existe, por lo que no la duplicamos
                if(j!=0){
                    $('.inner:last').clone(true, true).appendTo('.card:last');
                }
                //Añadimos el resto del contenido
                document.getElementsByClassName("card-image")[it].src=cards[i][1][j][1];
                document.getElementsByClassName("card-text")[it].innerHTML = cards[i][1][j][2];
                document.getElementsByClassName("card-date")[it].innerHTML = cards[i][1][j][3];
                it++;
            }
        }
        //Ocultamos la columna auxiliar
        $('.column:first').hide();
    });

    //Abrimos popup de borrar tarjeta
    $('div.inner').on('click', 'div.close', function() {
        modal.classList.toggle("show-modal");
    });

    //Funciones para abrir los popups
    function popup() {
        modal.classList.toggle("show-modal");
    };
    
    function popup2() {
        confirmation.classList.toggle("show-confirmation");
    };
    
    function popup3() {
        addition.classList.toggle("show-addition");
    };
    
    function share() {
        share.classList.toggle("show-share");
    };

    //Seleccionamos el popup que queremos abrir
    function windowOnClick(event) {
        if (event.target === modal) popup();
        else if (event.target === confirmation) popup2();
        else if (event.target === share) share();
        else if (event.target === addition){
            //Reset border
            $('#newText').css('border', 'solid rgb(158, 157, 157) 1px');
            popup3();
        }
    }

    //Abrir el popup de informacion desde cualquier tarjeta
    $('div.inner').on('click', 'div.trigger', function() {
        modal.classList.toggle("show-modal");
    });

    //Cerrar el popup de informacion en la x
    $('div.popup').on('click', 'button.close-button', function() {
        modal.classList.toggle("show-modal");
    });

    //Para clicar fuera del pop up y que se cierre
    window.addEventListener("click", windowOnClick);

    //Capture task that has been clicked
    var current_task;

    //Abrir el popup de confirmación desde cualquier x
    $('div.inner').on('click', 'button.close', function() {
        current_task = $(this).parent();
        confirmation.classList.toggle("show-confirmation");
    });

    //Cerrar el popup de confirmación en cancel
    $('div.confirmation').on('click', '#cancel', function() {
        confirmation.classList.toggle("show-confirmation");
    });

    //En confirm cerramos el popup y borramos la tarjeta
    $('div.confirmation').on('click', '#confirm', function() {
        current_task.remove();
        confirmation.classList.toggle("show-confirmation");
    });
    
    //Cerrar el popup de confirmación en la x
    $('div.popup-conf').on('click', 'button.close-button-confirmation', function() {
        confirmation.classList.toggle("show-confirmation");
    });

    //Abrir el popup de adición desde cualquier add task
    $('button.btn').on('click', function() {
        current_task = $(this).siblings();
        addition.classList.toggle("show-addition");
    });

    //Cerrar el popup de adición en cancel
    $('div.addition').on('click', '#cancel', function() {
        //Reset border
        $('#newText').css('border', 'solid rgb(158, 157, 157) 1px');
        addition.classList.toggle("show-addition");
    });
    
    //Cerrar el popup de adición en la x
    $('div.popup-add').on('click', 'button.close-button-addition', function() {
        addition.classList.toggle("show-addition");
    });
    
    //Cerrar el popup de compartir en la x
    $('div.popup-share').on('click', 'button.close-button-share', function() {
        share.classList.toggle("show-share");
    });
    
    //Dropdown de las columnas
    $('div.title').on('click', 'div.dropdown', function() {
        current_task = $(this).parent().parent().parent();
        $(this).children('div.dropdown-content').toggle("show-dropdown");
    });

    //Dropdown de usuario
    $('a#SO').on('click', function() {
        $('.dropbtn-user a.active').html("<img src='images/user.png' id='userImg'></img>" + 'Guest43636');
        $('p#Welcome').text('Welcome Guest43636!');
        window.open('login.html', '_self');
    });
    
    //Función para archivar las columnas
    $('div.title').on('click', 'a.archive', function() {
        current_task.remove();
    });
    
    //Abrir el popup de compartir
    $('div.inner').on('click', 'div.share_button', function() {
        current_task = $(this).parent().parent();
        share.classList.toggle("show-share");
    });

    //Al hacer like la iamgen se cambia
    $('div.inner').on('click', 'div.like_button', function() {
        /*
        if ($(this).attr('src') == 'images/like.svg'){
            $(this).parent().prepend('<input type="image" alt="like" id="lk" src="images/like-active.png" width="24" height="24" title="Show others you support this task">');
            $(this).remove();
        }else{
            $(this).parent().prepend('<input type="image" alt="like" id="lk" src="images/like.svg" width="24" height="24" title="Show others you support this task">');
            $(this).remove();
        }*/
        $(this).parent().prepend('<input type="image" alt="like" id="lk" src="images/like-active.png" width="24" height="24" title="Show others you support this task">');
        $(this).remove();
    });

    //Capturamos el día de hoy
    var today = new Date();
    var dd = String(today.getDate()).padStart(2, '0');
    var mm = String(today.getMonth() + 1).padStart(2, '0');
    var yyyy = today.getFullYear();

    today = dd + '-' + mm + '-' + yyyy;

    //En confirm cerramos el popup y borramos la tarjeta
    $('div.addition').on('click', '#confirm', function() {
        if($('#newText').val() == '' || $('#newText').val() == undefined){
            $('#newText').css('border', 'solid red 2px');
        }else{
            //Clonamos un inner
            current_task.children('.inner:last').clone(true, true).appendTo(current_task);
            //Añadimos el resto del contenido
            current_task.find('.card-text:last').html($('#newText').val());
            
            //Default value to new image
            var newImg = "images/office.jpg";
            //If image is provided set value
            if($('#newImg').val()!=""){
                newImg = $('#newImg').val();
            }
            current_task.find('.card-image:last').attr('src', newImg);
            
            current_task.find('.card-date:last').html(today);

            //Reset values
            $('#newText').val('');
            $('#newImg').val('');
            
            //Reset border
            $('#newText').css('border', 'solid rgb(158, 157, 157) 1px');

            //Salimos del popup
            addition.classList.toggle("show-addition");
        }
    });    
});
