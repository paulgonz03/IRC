#pragma once

# define SERVER_PREFIX      "cap_irc"
# define SERVER_VERSION     "1.0"

# define REPLY_WELCOME      "001"   /* Mensaje de bienvenida */
# define RPL_YOURHOST       "002"   /* Informa del servidor y version*/
# define RPL_CREATED        "003"   /* Informa cuando se creo el servidor*/
# define RPL_MYINFO         "004"   /* Que configuracion pueden tener los canales*/

# define NEED_MORE_PARAMS   "461"   /* Faltan parámetros en el comando */
# define PASSWORD_DISMATCH  "464"   /* Contraseña de servidor incorrecta */
# define NICKNAME_IN_USE    "433"   /* Nickname en uso por otro usuario */
# define ALREADY_REGISTERED "462"   /* Intento de autenticación ya autenticado */
# define UNKNOWN_COMMAND    "421"   /* Comando incorrecto/no encontrado */
