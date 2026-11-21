# Guía para Agentes (`AGENTS.md`)

## Propósito del proyecto
Simulador concurrente de colas de atención médica implementado en C con hilos POSIX (`pthreads`) y memoria compartida dentro de un mismo proceso. Modela la llegada de pacientes, clasificación en recepción por personal administrativo, derivación con prioridad a médicos generales, enfermería o especialistas (Cardiología, Neurología, Pediatría y Dermatología), abandono por espera prolongada, ajuste dinámico de personal administrativo y generación de un reporte final (`reporte_diario.txt`).

## Stack tecnológico
- **Lenguaje:** C (estándar C11 / POSIX.1-2008)
- **Concurrencia:** POSIX Threads (`pthread_mutex_t`, `pthread_cond_t`, `pthread_t`)
- **Compilador:** `cc` (`clang` en macOS / `gcc` en Linux)

## Estructura de carpetas
```text
.
├── .gitignore      # Exclusión de binarios, artefactos de ejecución y secretos
├── AGENTS.md       # Contexto y convenciones del repositorio para agentes
└── main.c          # Código fuente completo de la simulación concurrente
```

## Instalación, ejecución, pruebas y linting

### Compilación (Build)
```bash
cc -Wall -Wextra -Wpedantic -pthread main.c -o main
```

### Ejecución
```bash
# Velocidad normal (x1)
./main

# Velocidad acelerada (opciones soportadas: x1, x2, x4, x10)
./main x10
```
La simulación se ejecuta de forma continua hasta recibir `SIGINT` (`Ctrl+C`), momento en que despierta los hilos, escribe `reporte_diario.txt` y finaliza.

### Pruebas (Smoke test verificado)
No existe un framework de tests unitarios en esta rama; se verifica mediante ejecución controlada con señal `SIGINT`:
```bash
./main x10 &
SIM_PID=$!
sleep 2
kill -INT "$SIM_PID"
wait "$SIM_PID"
test -s reporte_diario.txt
```

### Linting / Verificación estática
```bash
cc -fsyntax-only -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wunused-macros -pthread main.c
```

## Convenciones de código
- Identificadores, comentarios y mensajes de salida en español (`snake_case` para funciones y variables, `PascalCase` para `typedef struct` y `typedef enum`, `UPPER_SNAKE_CASE` para macros).
- Declarar prototipos sin parámetros explícitamente con `(void)` en lugar de `()`.
- Proteger el acceso a colas y contadores compartidos con sus respectivos `pthread_mutex_t` (`cola->mutex`, `stats_mutex`, `print_mutex`).
- No versionar binarios compilados (`main`) ni reportes generados en tiempo de ejecución (`reporte_diario.txt`).

## Reglas de seguridad
- Los secretos o credenciales (si se integraran servicios externos) deben inyectarse exclusivamente mediante variables de entorno (`getenv`).
- Nunca escribir credenciales reales en el código fuente, commits, reportes ni `.env.example`.
- El archivo `.env` y cualquier llave o certificado (`*.pem`, `*.key`) nunca deben versionarse.

## Flujo Git
- Trabajar siempre en ramas `dev/*` y abrir cambios hacia `develop-AGENTS`.
- Nunca hacer commits ni push directo a `main` ni a `develop`.
- Usar mensajes de commit convencionales en inglés (`chore:`, `fix:`, `docs:`, `refactor:`).
