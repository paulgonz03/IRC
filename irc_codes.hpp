#pragma once

# define SERVER_PREFIX      "cap_irc"
# define SERVER_VERSION     "1.0"

# define REPLY_WELCOME      "001"   /* Mensaje de bienvenida */
# define RPL_YOURHOST       "002"   /* Informa del servidor y version*/
# define RPL_CREATED        "003"   /* Informa cuando se creo el servidor*/
# define RPL_MYINFO         "004"   /* Que configuracion pueden tener los canales*/

# define RPL_NOTOPIC        "331"   /* El canal existe pero no tiene topic */
# define RPL_TOPIC          "332"   /* Devuelve el topic actual de un canal */
# define RPL_NAMREPLY       "353"   /* Devuelve la lista de usuarios dentro del canal */
# define RPL_ENDOFNAMES     "366"   /* Marca el final de la lista de usuarios del canal */

# define INVALID_NAME       "403"   /* Nombre de canal invalido */
# define UNKNOWN_COMMAND    "421"   /* Comando incorrecto/no encontrado */
# define ERR_NONICKNAMEGIVEN "431"  /* Se ejecuta `NICK` sin proporcionar nickname */   
# define NICKNAME_IN_USE    "433"   /* Nickname en uso por otro usuario */
# define NOT_REGISTERED     "451"   /* Ejecucion de comandos sin estar autenticado */
# define NEED_MORE_PARAMS   "461"   /* Faltan parámetros en el comando */
# define ALREADY_REGISTERED "462"   /* Intento de autenticación ya autenticado */
# define PASSWORD_DISMATCH  "464"   /* Contraseña de servidor incorrecta */
# define ERR_CHANNELISFULL  "471"   /* El canal alcanzó el límite de usuarios (`+l`) */
# define ERR_INVITEONLYCHAN "473"   /* El canal requiere invitación (`+i`) y el usuario no la tiene */
# define ERR_BADCHANNELKEY  "475"    /* El canal requiere contraseña (`+k`) y no coincide o falta */
