*This project has been created as part of the 42 curriculum by camarcos.*

# ft_irc

## Description

`ft_irc` is a custom implementation of an IRC (Internet Relay Chat) server, written from
scratch in C++98. The goal of the project is to understand how a real-time, text-based
communication protocol works at the network level: managing multiple simultaneous TCP
clients with a single `poll()` call, parsing a line-oriented protocol out of a raw byte
stream, and reproducing the core behaviour of the IRC RFCs (2812) closely enough that a
real, unmodified IRC client can connect, authenticate, chat and manage channels against it.

The server does **not** implement client-to-client file transfer/bots (bonus) nor
server-to-server linking, and it never forks: every connection is handled through
non-blocking sockets multiplexed on a single `poll()`.

### Implemented features

- TCP/IPv4 server listening on a configurable port, protected by a connection password.
- Client registration: `PASS`, `NICK`, `USER`.
- Channels: `JOIN` (with support for channel keys and invite-only channels).
- Leaving: `PART`, `QUIT` (both graceful and abrupt client disconnects are handled and
  clean up channel membership/operator status correctly).
- Messaging: `PRIVMSG` / `NOTICE` to both users and channels, `PING`/`PONG`.
- Channel operators and regular users, with the operator-only commands:
  - `KICK` – eject a client from a channel.
  - `INVITE` – invite a client to a channel.
  - `TOPIC` – view or change a channel's topic (optionally operator-restricted).
  - `MODE` – channel modes `i` (invite-only), `t` (topic restricted to operators),
    `k` (channel key/password), `o` (operator privilege), `l` (user limit).
- Partial/fragmented TCP reads are buffered and reassembled per client before a command
  is processed (validated with tools such as `nc`, sending a command split across several
  writes).

## Instructions

### Compilation

```sh
make        # builds the ircserv binary
make clean  # removes object files
make fclean # removes object files and the binary
make re     # fclean + all
```

The project compiles with `c++` using `-Wall -Wextra -Werror -std=c++98`, with no
external/Boost libraries.

### Running the server

```sh
./ircserv <port> <password>
```

- `port`: the TCP port the server listens on.
- `password`: the connection password (`PASS`) required by clients before registering.

Example:

```sh
./ircserv 6667 mypassword
```

### Connecting with a client

The reference client used during development is [WeeChat](https://weechat.org/), tested
alongside raw `nc`/Python sockets for protocol-level and edge-case checks (partial
packets, abrupt disconnects, invalid input). To connect with WeeChat:

```
/server add ftirc 127.0.0.1/6667 -password=mypassword
/connect ftirc
```

Or with `netcat`, sending each line terminated by `\r\n`:

```sh
nc -C 127.0.0.1 6667
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #general
```

## Resources

- [RFC 1459 – Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 – Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [modern IRC client protocol documentation (ircdocs.horse)](https://modern.ircdocs.horse/)
- `man` pages for `socket`, `poll`, `fcntl`, `signal` (BSD/macOS man pages).

### AI usage

An AI assistant (Claude, via Claude Code) was used during this project as a coding aid,
under the constraints described in the subject's "AI Instructions" chapter:

- **Reviewing/extending an existing codebase**: the assistant was used to read through the
  already-written server (socket setup, poll loop, authentication, JOIN) and identify what
  was missing against the mandatory requirements (PRIVMSG, PART, QUIT, KICK, INVITE, TOPIC,
  MODE), as well as latent bugs (channel maps keyed by nickname becoming stale on `NICK`
  changes, channels not being cleaned up on client disconnect, and a missing `SIGPIPE`
  handler that could crash the whole server when writing to a socket the peer had already
  closed).
- **Implementing the missing commands**: the assistant proposed and wrote the
  implementation for the commands above, following the existing code's conventions
  (per-command source files, the `Client`/`Channel` static registries, the
  `sendMessage(prefix, command, arguments)` helper).
- **Testing**: the assistant wrote throwaway Python test scripts (not part of the
  submission) to exercise the protocol end-to-end — registration, channel join/part,
  private and channel messages, operator commands, channel modes, nickname changes,
  abrupt disconnects, and the `nc`-style fragmented-packet scenario described in the
  subject — and used them to find and fix the `SIGPIPE` crash mentioned above.

All AI-assisted code was read, understood and tested manually before being kept; the
resulting design decisions (protocol details, error codes, cleanup logic) were reviewed
against the RFCs listed above rather than taken at face value.
