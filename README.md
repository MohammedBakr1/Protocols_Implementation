# Protocols Implementation

HTTP and Gopher protocols implemented entirely from scratch in C, using the raw POSIX sockets API — no external networking libraries. Both a server and a client are implemented for each protocol.

This project was built as a hands-on introduction to socket programming: Gopher (RFC 1436) is one of the simplest possible TCP application protocols, and HTTP/1.1 (a practical subset) builds on the same socket primitives to introduce headers, status codes, and explicit content-length framing.

📄 **Full technical write-up (LaTeX/PDF):** [`docs/Protocols_Implementation.pdf`](docs/Protocols_Implementation.pdf) — a line-by-line walkthrough of every file, networking fundamentals, build instructions, and documented debugging sessions.

## Project Structure

```
Protocols_Implementation/
├── common/
│   ├── socket_utils.h      Shared socket helper declarations
│   └── socket_utils.c      Shared socket helper implementation
├── gopher/
│   ├── gopher_server.c     Gopher server
│   ├── gopher_client.c     Gopher client
│   └── gopher_root/        Sample content served by the Gopher server
├── http/
│   ├── http_server.c       HTTP server
│   ├── http_client.c       HTTP client
│   └── www/                Sample content served by the HTTP server
├── docs/                   LaTeX source + compiled PDF documentation
├── Makefile
└── LICENSE
```

## Building

Requires `gcc` and `make`.

```bash
# Debian/Ubuntu
sudo apt-get install -y build-essential

# Arch Linux
sudo pacman -S base-devel
```

```bash
make all
```

Produces four executables in the repo root: `gopher_server`, `gopher_client`, `http_server`, `http_client`.

## Running

**Gopher** (server must be launched from inside `gopher/`):

```bash
cd gopher
../gopher_server gopher_root &
cd ..
./gopher_client localhost 7070              # root menu
./gopher_client localhost 7070 about.txt    # a specific file
```

**HTTP** (server must be launched from inside `http/`):

```bash
cd http
../http_server www &
cd ..
./http_client localhost 8080 /
curl http://localhost:8080/
```

> The server's content directory is resolved relative to its current working directory — see [`docs/Protocols_Implementation.pdf`](docs/Protocols_Implementation.pdf), Chapter 12, for why.

## What's Implemented

| | Gopher | HTTP |
|---|---|---|
| Server | ✅ | ✅ (GET only) |
| Client | ✅ | ✅ |
| End-of-message signal | Connection close | `Content-Length` header |
| Status/error reporting | Informal type-`3` lines | 200 / 400 / 404 / 405 |
| Path traversal protection | ❌ | ✅ |

See the PDF documentation's final chapter for a full, honest list of limitations (no concurrency, no keep-alive, no chunked encoding, GET-only, etc.) and suggested extensions.

## License

MIT — see [LICENSE](LICENSE).
