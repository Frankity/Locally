# Locally - Servidor HTTP ligero

Locally es un servidor HTTP ligero escrito en C++ que permite servir archivos estáticos, exponer endpoints de API basados en archivos JSON y soporta recarga en vivo (live reload) para desarrollo web.

## Características

- Servidor HTTP simple y rápido.
- Soporte para WebSocket y recarga en vivo de páginas HTML.
- Configuración flexible mediante archivo `config.txt`.
- Endpoints de API dinámicos basados en archivos `.json`.
- Registro de logs en consola con diferentes niveles (info, warn, error, debug).
- Observador de archivos para detectar cambios en el directorio público.

## Estructura del proyecto

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

## Requisitos

- Windows (usa WinSock2 y API de Windows)
- [MinGW](https://www.mingw-w64.org/) o MSYS2 para compilar
- OpenSSL para soporte de WebSocket (SHA1 y Base64)
- Editor recomendado: Visual Studio Code

## Instalación y uso

1. **Clona el repositorio**  
   `git clone https://github.com/Frankity/Locally.git`

2. **Instala dependencias**  
   Asegúrate de tener MinGW/MSYS2 y OpenSSL instalados.

3. **Compila el proyecto**  
   Puedes usar el makefile:
   ```sh
   mingw32-make
   ```
   O desde VSCode usando la tarea `build`.

4. **Configura el servidor**  
   Edita el archivo `config.txt` para ajustar el puerto, rutas y otras opciones.

5. **Ejecuta el servidor**  
   ```sh
   ./locally.exe
   ```

6. **Accede desde tu navegador**  
   Abre [http://localhost:9090](http://localhost:9090) (o el puerto configurado).

## Configuración

El archivo `config.txt` permite definir:

- `port`: Puerto del servidor (por defecto 9090)
- `document_root`: Carpeta pública para archivos estáticos
- `api_root`: Carpeta donde están los endpoints de la API (archivos JSON)
- `live_reload`: Habilita/deshabilita recarga en vivo (`true`/`false`)
- `server_name`: Nombre del servidor
- `log_level`: Nivel de logs (`debug`, `info`, etc.)

## API

Los endpoints de la API se definen como archivos `.json` en la carpeta configurada (`api_root`).  
Puedes hacer peticiones GET a `/api/<nombre>` y filtrar resultados usando parámetros de consulta.

Ejemplo:
```
GET /api/products?category=teclado
```

## Licencia

MIT

---

Desarrollado por **Douglas Brunal**