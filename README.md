*This project has been created as part of the 42 curriculum by paulgonz03, camarcos & anadal-g*

# ft_irc

## Description

`ft_irc` is a custom IRC (Internet Relay Chat) server written in C++98. It implements a
single-threaded, non-blocking TCP server built around one `poll()` call, capable of
handling several clients at the same time without forking and without blocking on any
read/write operation.

A real IRC client (tested with `nc` and any RFC-compliant client) can connect to the
server, authenticate, pick a nickname and username, join channels, and exchange public
and private messages. Channels have regular users and operators; operators can manage
the channel through `KICK`, `INVITE`, `TOPIC` and `MODE` (`i`, `t`, `k`, `o`, `l`).

Implemented commands: `PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`, `PART`, `QUIT`,
`KICK`, `INVITE`, `TOPIC`, `MODE`.

## Instructions

### Compilation

```sh
make
```

Produces the `ircserv` executable. Other Makefile targets: `clean`, `fclean`, `re`.

### Execution

```sh
./ircserv <port> <password>
```

- `port`: TCP port the server listens on.
- `password`: password required by clients to authenticate (`PASS` command).

### Connecting

With any IRC client, or for quick manual testing with `nc`:

```sh
nc -C 127.0.0.1 <port>
PASS <password>
NICK <nickname>
USER <username> 0 * :<realname>
JOIN #channel
```

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- `man poll`, `man socket`, `man fcntl` (non-blocking I/O reference)

### AI usage

An AI assistant (Claude, via Claude Code) was used during this project as a coding aid,
mainly for:

- Diagnosing and fixing a circular-include issue between `Client.hpp`, `Server.hpp` and
  `Channel.hpp` that produced misleading IntelliSense errors.
- Implementing the channel-operator commands (`KICK`, `INVITE`, `TOPIC`, `MODE`) and the
  channel-leaving logic (`PART`, `QUIT`, and cleanup on abrupt disconnection), following
  the existing code style and command dispatch pattern already used for `PASS`/`NICK`/
  `USER`/`JOIN`.

All AI-suggested code was reviewed, tested manually against the running server (multiple
simultaneous connections, including edge cases like abrupt disconnects and permission
checks), and understood before being kept in the project.
