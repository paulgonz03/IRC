#pragma once

# define SERVER_PREFIX      "cap_irc"
# define SERVER_VERSION     "1.0"

# define REPLY_WELCOME      "001"   /* Mensaje de bienvenida */
# define RPL_YOURHOST       "002"   /* Informa del servidor y version*/
# define RPL_CREATED        "003"   /* Informa cuando se creo el servidor*/
# define RPL_MYINFO         "004"   /* Que configuracion pueden tener los canales*/

# define RPL_NOTOPIC        "331"   /* El canal existe pero no tiene topic */
# define RPL_TOPIC          "332"   /* Devuelve el topic actual de un canal */
# define RPL_INVITING       "341"   /* Confirma al que invita que la invitacion se envio */
# define RPL_NAMREPLY       "353"   /* Devuelve la lista de usuarios dentro del canal */
# define RPL_ENDOFNAMES     "366"   /* Marca el final de la lista de usuarios del canal */
# define RPL_CHANNELMODEIS  "324"   /* Devuelve los modos actuales de un canal */

# define INVALID_NAME       "403"   /* Nombre de canal invalido */
# define ERR_NOSUCHNICK     "401"   /* No existe ese nickname */
# define ERR_NOSUCHCHANNEL  "403"   /* No existe ese canal */
# define ERR_CANNOTSENDTOCHAN "404" /* No se puede enviar mensaje al canal (no estas dentro) */
# define ERR_NORECIPIENT    "411"   /* PRIVMSG sin destinatario */
# define ERR_NOTEXTTOSEND   "412"   /* PRIVMSG sin texto */
# define UNKNOWN_COMMAND    "421"   /* Comando incorrecto/no encontrado */
# define ERR_NONICKNAMEGIVEN "431"  /* Se ejecuta `NICK` sin proporcionar nickname */
# define NICKNAME_IN_USE    "433"   /* Nickname en uso por otro usuario */
# define ERR_USERNOTINCHANNEL "441" /* El usuario objetivo no esta en el canal */
# define ERR_NOTONCHANNEL   "442"   /* El cliente no esta dentro del canal */
# define ERR_USERONCHANNEL  "443"   /* El usuario ya esta dentro del canal (INVITE) */
# define NOT_REGISTERED     "451"   /* Ejecucion de comandos sin estar autenticado */
# define NEED_MORE_PARAMS   "461"   /* Faltan parámetros en el comando */
# define ALREADY_REGISTERED "462"   /* Intento de autenticación ya autenticado */
# define PASSWORD_DISMATCH  "464"   /* Contraseña de servidor incorrecta */
# define ERR_CHANNELISFULL  "471"   /* El canal alcanzó el límite de usuarios (`+l`) */
# define ERR_UNKNOWNMODE    "472"   /* Modo de canal desconocido */
# define ERR_INVITEONLYCHAN "473"   /* El canal requiere invitación (`+i`) y el usuario no la tiene */
# define ERR_BADCHANNELKEY  "475"    /* El canal requiere contraseña (`+k`) y no coincide o falta */
# define ERR_CHANOPRIVSNEEDED "482" /* Se requiere ser operador del canal */
