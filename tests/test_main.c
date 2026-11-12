/*
 * Pruebas unitarias de main.c.
 * Se incluye el fuente directamente renombrando main() para poder
 * acceder a las funciones y variables globales sin modificar el programa.
 */
#define main programa_main
#include "../main.c"
#undef main


static int pruebas = 0;
static int fallos = 0;

#define CHECK(cond) do { \
    pruebas++; \
    if (!(cond)) { \
        fallos++; \
        fprintf(stderr, "FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static Paciente paciente(int id, int prioridad) {
    Paciente p = {0};
    p.id = id;
    p.prioridad = prioridad;
    return p;
}

static void test_fifo(void) {
    static Cola c;
    init_cola(&c);
    for (int i = 1; i <= 5; i++) enqueue(&c, paciente(i, 3));
    CHECK(c.count == 5);
    for (int i = 1; i <= 5; i++) CHECK(dequeue(&c).id == i);
    CHECK(c.count == 0);
}

static void test_cola_llena_descarta(void) {
    static Cola c;
    init_cola(&c);
    for (int i = 1; i <= MAX_PACIENTES + 10; i++) enqueue(&c, paciente(i, 1));
    CHECK(c.count == MAX_PACIENTES);
    CHECK(dequeue(&c).id == 1);
    enqueue(&c, paciente(9999, 1));
    CHECK(c.count == MAX_PACIENTES);
}

static void test_circular_wraparound(void) {
    static Cola c;
    init_cola(&c);
    /* Llevar los índices cerca del final del arreglo */
    for (int i = 0; i < MAX_PACIENTES - 2; i++) {
        enqueue(&c, paciente(1, 1));
        dequeue(&c);
    }
    CHECK(c.frente == MAX_PACIENTES - 2);
    enqueue_prioridad(&c, paciente(10, 3));
    enqueue_prioridad(&c, paciente(11, 4));
    enqueue_prioridad(&c, paciente(12, 1)); /* debe quedar primero */
    enqueue_prioridad(&c, paciente(13, 2));
    CHECK(c.count == 4);
    CHECK(dequeue(&c).id == 12);
    CHECK(dequeue(&c).id == 13);
    CHECK(dequeue(&c).id == 10);
    CHECK(dequeue(&c).id == 11);
}

static void test_prioridad_orden_y_estabilidad(void) {
    static Cola c;
    init_cola(&c);
    int prioridades[] = {3, 1, 5, 1, 2, 3, 5, 4};
    int n = (int)(sizeof(prioridades) / sizeof(prioridades[0]));
    for (int i = 0; i < n; i++) enqueue_prioridad(&c, paciente(i + 1, prioridades[i]));
    CHECK(c.count == n);
    /* Orden esperado: prioridad ascendente, FIFO entre iguales */
    int esperado[] = {2, 4, 5, 1, 6, 8, 3, 7};
    for (int i = 0; i < n; i++) CHECK(dequeue(&c).id == esperado[i]);
}

static void test_dequeue_vacia_sin_simulacion(void) {
    static Cola c;
    init_cola(&c);
    simulacion_activa = 0;
    CHECK(dequeue(&c).id == 0); /* no debe bloquear */
    simulacion_activa = 1;
}

static Cola cola_hilos;
static void *productor(void *arg) {
    (void)arg;
    for (int i = 1; i <= 200; i++) enqueue(&cola_hilos, paciente(i, 1));
    return NULL;
}

static void test_productor_consumidor(void) {
    init_cola(&cola_hilos);
    pthread_t t;
    pthread_create(&t, NULL, productor, NULL);
    int ok = 1;
    for (int i = 1; i <= 200; i++) {
        if (dequeue(&cola_hilos).id != i) ok = 0;
    }
    pthread_join(t, NULL);
    CHECK(ok);
    CHECK(cola_hilos.count == 0);
}

static void test_verificar_abandono(void) {
    SPEED_FACTOR = 1;
    srand(1234);
    /* Espera recién iniciada: nunca abandona */
    int abandonos = 0;
    for (int i = 0; i < 1000; i++) abandonos += verificar_abandono(time(NULL));
    CHECK(abandonos == 0);

    /* Espera de 30 minutos: probabilidad ~40% */
    abandonos = 0;
    for (int i = 0; i < 2000; i++) abandonos += verificar_abandono(time(NULL) - 1800);
    CHECK(abandonos > 600 && abandonos < 1000);

    /* Con factor x10, 2 minutos reales = 20 minutos simulados (>15 min, ~25%) */
    SPEED_FACTOR = 10;
    abandonos = 0;
    for (int i = 0; i < 2000; i++) abandonos += verificar_abandono(time(NULL) - 100);
    CHECK(abandonos > 350 && abandonos < 650);
    SPEED_FACTOR = 1;
}

static void test_dormir_simulacion_acelerada(void) {
    SPEED_FACTOR = 10;
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    dormir_simulacion(2); /* 0.2 s reales */
    dormir_simulacion(0); /* no duerme */
    clock_gettime(CLOCK_MONOTONIC, &b);
    double s = (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
    CHECK(s >= 0.19 && s < 1.0);
    SPEED_FACTOR = 1;
}

static void test_generar_reporte(void) {
    char dir[] = "/tmp/test_reporte_XXXXXX";
    char previo[4096];
    CHECK(getcwd(previo, sizeof previo) != NULL);
    CHECK(mkdtemp(dir) != NULL);
    CHECK(chdir(dir) == 0);

    init_cola(&cola_recepcion);
    init_cola(&cola_medico_general);
    init_cola(&cola_enfermeria);
    for (int i = 0; i < 4; i++) init_cola(&cola_especialista[i]);
    enqueue(&cola_recepcion, paciente(1, 1));
    enqueue(&cola_medico_general, paciente(2, 1));
    total_pacientes_generados = 4;
    total_pacientes_atendidos = 1;
    tiempo_inicio_simulacion = time(NULL);

    fflush(stdout);
    generar_reporte();

    FILE *f = fopen("reporte_diario.txt", "r");
    CHECK(f != NULL);
    if (f) {
        char buf[8192];
        size_t n = fread(buf, 1, sizeof buf - 1, f);
        buf[n] = '\0';
        fclose(f);
        CHECK(strstr(buf, "Pacientes generados: 4") != NULL);
        CHECK(strstr(buf, "Pacientes no contabilizados (en recepción): 1") != NULL);
        CHECK(strstr(buf, "Pacientes sin atención (clasificados, no atendidos): 1") != NULL);
        CHECK(strstr(buf, "Eficiencia del sistema: 25.00%") != NULL);
    }
    remove("reporte_diario.txt");
    CHECK(chdir(previo) == 0);
    rmdir(dir);
}

int main(void) {
    test_fifo();
    test_cola_llena_descarta();
    test_circular_wraparound();
    test_prioridad_orden_y_estabilidad();
    test_dequeue_vacia_sin_simulacion();
    test_productor_consumidor();
    test_verificar_abandono();
    test_dormir_simulacion_acelerada();
    test_generar_reporte();

    printf("\n%d comprobaciones, %d fallos\n", pruebas, fallos);
    return fallos == 0 ? 0 : 1;
}
