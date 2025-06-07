# Locally - Lightweight HTTP Server

Locally is a lightweight HTTP server written in C++ that allows you to serve static files, expose API endpoints based on JSON files, and supports live reload for web development.

## Features

- Simple and fast HTTP server.
- WebSocket support and live reload for HTML pages.
- Flexible configuration via `config.txt` file.
- Dynamic API endpoints based on `.json` files.
- Console logging with different levels (info, warn, error, debug).
- File watcher to detect changes in the public directory.

## Project Structure

```
.
├── main.cpp
├── config.txt
├── makefile
├── include/
├── src/
├── public/
├── api/
└── .vscode/
```

## Requirements

- Windows (uses WinSock2 and Windows API)
- [MinGW](https://www.mingw-w64.org/) or MSYS2 for compilation
- OpenSSL for WebSocket support (SHA1 and Base64)
- Recommended editor: Visual Studio Code

## Installation and Usage

1. **Clone the repository**  
   `git clone https://github.com/Frankity/Locally.git`

2. **Install dependencies**  
   Make sure you have MinGW/MSYS2 and OpenSSL installed.

3. **Compile the project**  
   You can use the makefile:
   ```sh
   mingw32-make
   ```
   Or from VSCode using the `build` task.

4. **Configure the server**  
   Edit the `config.txt` file to adjust the port, paths, and other options.

5. **Run the server**  
   ```sh
   ./locally.exe
   ```

6. **Access from your browser**  
   Open [http://localhost:9090](http://localhost:9090) (or the configured port).

## Configuration

The `config.txt` file allows you to define:

- `port`: Server port (default 9090)
- `document_root`: Public folder for static files
- `api_root`: Folder where API endpoints are located (JSON files)
- `live_reload`: Enable/disable live reload (`true`/`false`)
- `server_name`: Server name
- `log_level`: Log level (`debug`, `info`, etc.)

## API

API endpoints are defined as `.json` files in the configured folder (`api_root`).  
You can make GET requests to `/api/<name>` and filter results using query parameters.

Example:
```
GET /api/products?category=keyboard
```

## License

MIT

---

Developed by **Douglas Brunal**