# simulacion-memoria-compartida-c

[![CI](https://github.com/Demiannnnnn/simulacion-memoria-compartida-c/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/Demiannnnnn/simulacion-memoria-compartida-c/actions/workflows/ci.yml)

> **Nota aclaratoria sobre la arquitectura**: Pese al nombre del repositorio (`simulacion-memoria-compartida-c`), la simulación está implementada utilizando **hilos POSIX (`pthreads`)** que comparten memoria dentro del espacio de direcciones de un **mismo proceso**, y **no** mediante mecanismos de memoria compartida entre procesos del sistema operativo (`shm` / IPC como `shmget` o `mmap`).

## Descripción

Simulación concurrente en C del flujo de atención de pacientes en un centro médico. El sistema modela:
- **Generación de pacientes**: llegadas aleatorias con diferentes prioridades (1 a 5) y tipos de atención (Medicina General, Enfermería y Especialidades).
- **Personal Administrativo**: recepción, clasificación y triaje de pacientes hacia las colas de atención correspondientes, con ajuste dinámico de personal activo según la carga.
- **Personal Médico**: atención de consultas por médicos generales, enfermeras y especialistas (Cardiología, Neurología, Pediatría y Dermatología).
- **Monitoreo y Reporte**: hilos auxiliares que supervisan el estado de las colas, tiempo transcurrido y generación de reporte diario (`reporte_diario.txt`).

## Stack

- **Lenguaje**: C (estándar GNU11 / C11)
- **Hilos y Sincronización**: POSIX Threads (`pthreads`), candados (`pthread_mutex_t`) y variables de condición (`pthread_cond_t`)
- **Compilador**: GCC / Clang
- **Automatización**: GNU Make
- **CI**: GitHub Actions

## Cómo compilar y ejecutar

### Compilación

Para compilar el ejecutable principal:

```bash
make all CFLAGS="-std=gnu11 -Wall -Wextra -Werror -O2"
```

O simplemente:

```bash
make
```

### Ejecución

El programa acepta un parámetro opcional con el factor de aceleración de tiempo (`x1`, `x2`, `x4`, `x10`):

```bash
# Velocidad normal (1x)
./main

# Simulación acelerada 10x
./main x10
```

Para detener la simulación de forma ordenada, envía una señal de interrupción presionando `Ctrl+C` (`SIGINT`). El sistema completará la jornada y generará el archivo `reporte_diario.txt`.

## Cómo correr las pruebas

Las pruebas unitarias y de concurrencia se compilan y ejecutan con Make:

```bash
make test
```

Para limpiar los ejecutables y artefactos de compilación:

```bash
make clean
```
