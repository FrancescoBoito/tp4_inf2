#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ARCHIVO_RANKING "ranking.txt"
#define ARCHIVO_PARTIDA "partida_guardada.txt"
#define MAX_NOMBRE 50
#define MAX_RANKING 5

/*
    BATALLA NAVAL - TRABAJO PRACTICO 5 - PARTE A

    Este programa parte del TP4 y conserva sus funcionalidades.

    CAMBIO PRINCIPAL DEL TP5:
    El ranking ya no se guarda en un arreglo dinamico de estructuras.
    Ahora se administra mediante una LISTA SIMPLEMENTE ENLAZADA.

    Cada nodo contiene:
    - un struct record con nombre, disparos y fecha;
    - un puntero al siguiente nodo.

    La lista se mantiene siempre ordenada de menor a mayor cantidad de
    disparos. Como menos disparos significa un mejor resultado, el primer
    nodo de la lista es siempre el mejor jugador.

    Ademas, el ranking se limita a los 5 mejores resultados. Cada vez que se
    inserta un nuevo jugador se lo coloca en su posicion correcta. Si despues
    de la insercion hay mas de 5 nodos, se elimina todo lo que quede despues
    del quinto nodo.

    ranking.txt conserva el mismo formato usado en el TP4:

        nombre;disparos;AAAA-MM-DD HH:MM:SS

    Al iniciar el programa se reconstruye la lista desde ranking.txt.
    Al terminar una partida y al salir del programa se recorre la lista y se
    vuelve a escribir ranking.txt con los 5 mejores resultados.
*/

/* ======================= ESTRUCTURAS ======================= */

/*
    Representa los datos de un jugador del ranking.

    Se conserva fecha del TP4 para seguir mostrando cuanto tiempo lleva
    vigente el record.
*/
struct record
{
    char nombre[MAX_NOMBRE];
    int disparos;
    time_t fecha;
};

/*
    Nodo de la lista simplemente enlazada pedida por el TP5.

    jugador   -> contiene los datos del ranking.
    siguiente -> apunta al proximo nodo de la lista.

    Si siguiente vale NULL significa que este nodo es el ultimo.
*/
struct nodo_ranking
{
    struct record jugador;
    struct nodo_ranking *siguiente;
};

/*
    Representa un disparo realizado durante una partida.

    resultado contiene:
    - "agua" si el disparo no acerto;
    - "hundido" si el disparo encontro el barco.
*/
struct disparo
{
    int fila;
    int columna;
    char resultado[10];
};

/* ======================= PROTOTIPOS ======================= */

void inicializarTablero(char *tablero, int filas, int columnas);

void mostrarTablero(char *tablero, int filas, int columnas,
                    int cantidadDisparos);

void mostrarTableroFinal(char *tablero, int filas, int columnas,
                         int filaBarco, int colBarco,
                         int cantidadDisparos);

void generarBarco(int *filaBarco, int *colBarco,
                  int filas, int columnas);

int disparar(char *tablero, int filas, int columnas,
             int filaBarco, int colBarco,
             int *cantidadDisparos,
             struct disparo **historial);

void mostrarHistorial(struct disparo *historial,
                      int cantidadDisparos);

/* Funciones nuevas/cambiadas para trabajar con la lista del ranking. */
int insertarRanking(struct nodo_ranking **ranking,
                    char nombre[], int disparos, time_t fecha);

void limitarRankingTop5(struct nodo_ranking *ranking);

void mostrarRanking(struct nodo_ranking *ranking);

void cargarRanking(struct nodo_ranking **ranking);

int guardarRanking(struct nodo_ranking *ranking);

void mostrarMejorJugador(struct nodo_ranking *ranking);

void liberarRanking(struct nodo_ranking **ranking);

int guardarHistorialJugador(char nombre[],
                            struct disparo *historial,
                            int cantidadDisparos);

void crearNombreArchivoDisparos(char nombre[], char nombreArchivo[]);

int guardarPartida(char *tablero, int filas, int columnas,
                   int filaBarco, int colBarco,
                   int cantidadDisparos,
                   struct disparo *historial,
                   int barcoHundido);

int cargarPartida(char **tablero,
                  int *filas, int *columnas,
                  int *filaBarco, int *colBarco,
                  int *cantidadDisparos,
                  struct disparo **historial,
                  int *barcoHundido);

void prepararNuevaPartida(char **tablero,
                          struct disparo **historial,
                          int *cantidadDisparos,
                          int *juegoIniciado,
                          int *barcoHundido);

void sanitizarNombre(char nombre[]);

void liberarMemoria(char *tablero, struct disparo *historial);

void limpiarBuffer(void);

/* ======================= MAIN ======================= */

int main(void)
{
    char *tablero = NULL;

    int filas = 0;
    int columnas = 0;

    int filaBarco = 0;
    int colBarco = 0;

    int cantidadDisparos = 0;
    struct disparo *historial = NULL;

    /*
        ranking apunta al PRIMER nodo de la lista.

        Al comenzar vale NULL porque todavia no existe ningun nodo en memoria.
        Ya no hace falta cantidadRanking ni un arreglo de struct record.
    */
    struct nodo_ranking *ranking = NULL;

    int opcion = 0;
    int juegoIniciado = 0;
    int barcoHundido = 0;

    srand((unsigned int) time(NULL));

    /*
        Reconstruye la lista enlazada a partir del archivo ranking.txt.
        Si el archivo no existe, ranking continua valiendo NULL.
    */
    cargarRanking(&ranking);

    do
    {
        printf("\n=========================================\n");
        printf("              BATALLA NAVAL\n");
        printf("=========================================\n");

        /* El primer nodo es siempre el mejor jugador. */
        mostrarMejorJugador(ranking);

        printf("\n1. Iniciar juego nuevo\n");
        printf("2. Continuar juego guardado\n");
        printf("3. Mostrar tablero\n");
        printf("4. Disparar\n");
        printf("5. Mostrar historial de disparos\n");
        printf("6. Guardar juego actual\n");
        printf("7. Mostrar Top 5\n");
        printf("8. Salir\n");
        printf("Opcion: ");

        if (scanf("%d", &opcion) != 1)
        {
            printf("\nDebe ingresar un numero.\n");
            limpiarBuffer();
            opcion = 0;
            continue;
        }

        switch (opcion)
        {
            case 1:
                prepararNuevaPartida(&tablero, &historial,
                                     &cantidadDisparos,
                                     &juegoIniciado,
                                     &barcoHundido);

                printf("\nIngrese cantidad de filas: ");
                if (scanf("%d", &filas) != 1)
                {
                    printf("\nCantidad de filas incorrecta.\n");
                    limpiarBuffer();
                    break;
                }

                printf("Ingrese cantidad de columnas: ");
                if (scanf("%d", &columnas) != 1)
                {
                    printf("\nCantidad de columnas incorrecta.\n");
                    limpiarBuffer();
                    break;
                }

                if (filas <= 0 || columnas <= 0)
                {
                    printf("\nEl tablero debe tener filas y columnas mayores que cero.\n");
                    break;
                }

                tablero = (char *) malloc(filas * columnas * sizeof(char));

                if (tablero == NULL)
                {
                    printf("\nNo se pudo reservar memoria para el tablero.\n");
                    break;
                }

                inicializarTablero(tablero, filas, columnas);
                generarBarco(&filaBarco, &colBarco, filas, columnas);

                juegoIniciado = 1;

                printf("\nJuego iniciado con tablero de %d x %d.\n",
                       filas, columnas);

                mostrarTablero(tablero, filas, columnas,
                               cantidadDisparos);
                break;

            case 2:
                if (cargarPartida(&tablero,
                                  &filas, &columnas,
                                  &filaBarco, &colBarco,
                                  &cantidadDisparos,
                                  &historial,
                                  &barcoHundido))
                {
                    juegoIniciado = 1;
                    printf("\nLa partida guardada fue cargada correctamente.\n");
                    mostrarTablero(tablero, filas, columnas,
                                   cantidadDisparos);
                }
                else
                {
                    printf("\nNo se pudo cargar una partida guardada.\n");
                }
                break;

            case 3:
                if (juegoIniciado == 0)
                {
                    printf("\nPrimero debe iniciar o cargar un juego.\n");
                }
                else
                {
                    mostrarTablero(tablero, filas, columnas,
                                   cantidadDisparos);
                }
                break;

            case 4:
                if (juegoIniciado == 0)
                {
                    printf("\nPrimero debe iniciar o cargar un juego.\n");
                }
                else if (barcoHundido == 1)
                {
                    printf("\nEl juego ya termino. Inicie una nueva partida.\n");
                }
                else
                {
                    barcoHundido = disparar(tablero, filas, columnas,
                                            filaBarco, colBarco,
                                            &cantidadDisparos,
                                            &historial);

                    if (barcoHundido == 1)
                    {
                        char nombre[MAX_NOMBRE];
                        time_t fechaResultado;

                        printf("\nGanaste en %d disparos.\n",
                               cantidadDisparos);

                        mostrarTableroFinal(tablero, filas, columnas,
                                            filaBarco, colBarco,
                                            cantidadDisparos);

                        mostrarHistorial(historial, cantidadDisparos);

                        printf("\nIngrese su nombre: ");
                        limpiarBuffer();
                        fgets(nombre, MAX_NOMBRE, stdin);
                        nombre[strcspn(nombre, "\n")] = '\0';

                        sanitizarNombre(nombre);

                        if (strlen(nombre) == 0)
                        {
                            strcpy(nombre, "Jugador");
                        }

                        fechaResultado = time(NULL);

                        /*
                            TP5:

                            1. Se crea un nodo dinamicamente.
                            2. Se lo inserta en orden ascendente por disparos.
                            3. Si la lista supera 5 elementos, se elimina el
                               que quede despues del quinto.
                        */
                        if (insertarRanking(&ranking,
                                            nombre,
                                            cantidadDisparos,
                                            fechaResultado))
                        {
                            /*
                                La consigna pide recorrer la lista y guardar
                                sus elementos al finalizar una partida.
                            */
                            if (guardarRanking(ranking))
                            {
                                printf("\nTop 5 actualizado en ranking.txt.\n");
                            }
                            else
                            {
                                printf("\nNo se pudo guardar ranking.txt.\n");
                            }
                        }
                        else
                        {
                            printf("\nNo se pudo reservar memoria para el nuevo nodo del ranking.\n");
                        }

                        if (guardarHistorialJugador(nombre,
                                                    historial,
                                                    cantidadDisparos))
                        {
                            printf("Historial de disparos guardado.\n");
                        }
                        else
                        {
                            printf("No se pudo guardar el historial de disparos.\n");
                        }

                        /*
                            Una partida terminada ya no debe quedar disponible
                            como partida pendiente para continuar.
                        */
                        remove(ARCHIVO_PARTIDA);
                    }
                    else
                    {
                        mostrarTablero(tablero, filas, columnas,
                                       cantidadDisparos);
                    }
                }
                break;

            case 5:
                if (juegoIniciado == 0)
                {
                    printf("\nPrimero debe iniciar o cargar un juego.\n");
                }
                else
                {
                    mostrarHistorial(historial, cantidadDisparos);
                }
                break;

            case 6:
                if (juegoIniciado == 0)
                {
                    printf("\nNo hay un juego iniciado para guardar.\n");
                }
                else if (barcoHundido == 1)
                {
                    printf("\nLa partida ya termino y no necesita guardarse.\n");
                }
                else if (guardarPartida(tablero, filas, columnas,
                                        filaBarco, colBarco,
                                        cantidadDisparos,
                                        historial,
                                        barcoHundido))
                {
                    printf("\nPartida guardada en %s.\n", ARCHIVO_PARTIDA);
                }
                else
                {
                    printf("\nNo se pudo guardar la partida.\n");
                }
                break;

            case 7:
                mostrarRanking(ranking);
                break;

            case 8:
                printf("\nFin del programa.\n");
                break;

            default:
                printf("\nOpcion incorrecta.\n");
                break;
        }

    } while (opcion != 8);

    /*
        El TP5 pide guardar nuevamente la lista al salir.
        Se abre ranking.txt en modo "w", por lo que el archivo queda formado
        solamente por los nodos que actualmente pertenecen al Top 5.
    */
    if (!guardarRanking(ranking))
    {
        printf("\nNo se pudo guardar el ranking al salir.\n");
    }

    /* Memoria dinamica de la partida. */
    liberarMemoria(tablero, historial);

    /*
        Memoria dinamica del ranking.
        Se libera NODO POR NODO para evitar memory leaks.
    */
    liberarRanking(&ranking);

    return 0;
}

/* ======================= FUNCIONES DEL JUEGO ======================= */

/*
    Coloca un punto en cada posicion del tablero para indicar que todavia
    no se disparo en ninguna celda.
*/
void inicializarTablero(char *tablero, int filas, int columnas)
{
    int i;
    int total = filas * columnas;

    for (i = 0; i < total; i++)
    {
        *(tablero + i) = '.';
    }
}

/*
    Muestra el tablero sin revelar la posicion del barco.

    El tablero esta guardado como un arreglo lineal. Para acceder a la fila i
    y la columna j se calcula la posicion:

        i * columnas + j
*/
void mostrarTablero(char *tablero, int filas, int columnas,
                    int cantidadDisparos)
{
    int i, j;

    printf("\nTablero:\n\n");

    printf("   ");
    for (j = 0; j < columnas; j++)
    {
        printf("%2d ", j);
    }
    printf("\n");

    for (i = 0; i < filas; i++)
    {
        printf("%2d ", i);

        for (j = 0; j < columnas; j++)
        {
            printf(" %c ", *(tablero + i * columnas + j));
        }
        printf("\n");
    }

    printf("\nDisparos realizados: %d\n", cantidadDisparos);
}

/*
    Al terminar la partida se coloca una B en la posicion del barco y se
    muestra el tablero completo.
*/
void mostrarTableroFinal(char *tablero, int filas, int columnas,
                         int filaBarco, int colBarco,
                         int cantidadDisparos)
{
    *(tablero + filaBarco * columnas + colBarco) = 'B';

    printf("\nTablero final:\n");
    mostrarTablero(tablero, filas, columnas, cantidadDisparos);
}

/*
    Genera una fila y una columna aleatorias dentro del tablero.

    Los parametros filaBarco y colBarco son punteros porque la funcion debe
    modificar las variables originales declaradas en main.
*/
void generarBarco(int *filaBarco, int *colBarco,
                  int filas, int columnas)
{
    *filaBarco = rand() % filas;
    *colBarco = rand() % columnas;
}

/*
    Solicita una posicion de disparo, controla que sea valida y la agrega al
    historial dinamico.

    Como la cantidad de disparos no se conoce de antemano, realloc aumenta el
    arreglo historial una posicion cada vez que se realiza un disparo valido.

    Retorna:
    - 1 si el barco fue hundido;
    - 0 si el disparo fue agua o no pudo realizarse.
*/
int disparar(char *tablero, int filas, int columnas,
             int filaBarco, int colBarco,
             int *cantidadDisparos,
             struct disparo **historial)
{
    int f, c;
    int nuevaCantidad;
    struct disparo *auxiliar;

    printf("\nFila: ");
    if (scanf("%d", &f) != 1)
    {
        printf("\nFila incorrecta.\n");
        limpiarBuffer();
        return 0;
    }

    printf("Columna: ");
    if (scanf("%d", &c) != 1)
    {
        printf("\nColumna incorrecta.\n");
        limpiarBuffer();
        return 0;
    }

    if (f < 0 || f >= filas || c < 0 || c >= columnas)
    {
        printf("\nDisparo fuera del tablero.\n");
        return 0;
    }

    if (*(tablero + f * columnas + c) == 'A' ||
        *(tablero + f * columnas + c) == 'B')
    {
        printf("\nYa disparaste en esa posicion.\n");
        return 0;
    }

    nuevaCantidad = *cantidadDisparos + 1;

    auxiliar = (struct disparo *) realloc(
        *historial,
        nuevaCantidad * sizeof(struct disparo));

    if (auxiliar == NULL)
    {
        printf("\nNo se pudo reservar memoria para guardar el disparo.\n");
        return 0;
    }

    *historial = auxiliar;

    (*historial)[nuevaCantidad - 1].fila = f;
    (*historial)[nuevaCantidad - 1].columna = c;

    *cantidadDisparos = nuevaCantidad;

    if (f == filaBarco && c == colBarco)
    {
        strcpy((*historial)[nuevaCantidad - 1].resultado, "hundido");
        *(tablero + f * columnas + c) = 'B';

        printf("\nImpacto. Barco hundido.\n");
        return 1;
    }

    strcpy((*historial)[nuevaCantidad - 1].resultado, "agua");
    *(tablero + f * columnas + c) = 'A';

    printf("\nAgua.\n");
    return 0;
}

/*
    Recorre el arreglo de estructuras y muestra fila, columna y resultado de
    cada disparo.
*/
void mostrarHistorial(struct disparo *historial,
                      int cantidadDisparos)
{
    int i;

    if (cantidadDisparos == 0)
    {
        printf("\nTodavia no se realizaron disparos.\n");
        return;
    }

    printf("\nHistorial de disparos:\n");

    for (i = 0; i < cantidadDisparos; i++)
    {
        printf("Disparo %d -> fila %d, columna %d, resultado: %s\n",
               i + 1,
               historial[i].fila,
               historial[i].columna,
               historial[i].resultado);
    }
}

/* ======================= FUNCIONES DEL RANKING ======================= */

/*
    Inserta un jugador en la lista manteniendo el orden ascendente por
    cantidad de disparos.

    Se usa un DOBLE PUNTERO (struct nodo_ranking **) porque la funcion puede
    necesitar cambiar el primer nodo de la lista.

    Ejemplo:

        4 -> 8 -> 12

    si llega un jugador con 6 disparos, el resultado debe ser:

        4 -> 6 -> 8 -> 12

    Si el nuevo jugador es mejor que todos, se inserta al principio.

    malloc reserva memoria para UN nuevo nodo. Cada nodo se reserva por
    separado: esta es la diferencia principal respecto del ranking del TP4,
    donde todos los records estaban dentro de un arreglo agrandado con realloc.
*/
int insertarRanking(struct nodo_ranking **ranking,
                    char nombre[], int disparos, time_t fecha)
{
    struct nodo_ranking *nuevo;
    struct nodo_ranking *actual;

    nuevo = (struct nodo_ranking *) malloc(sizeof(struct nodo_ranking));

    if (nuevo == NULL)
    {
        return 0;
    }

    /* Se cargan los datos del jugador dentro del nuevo nodo. */
    strcpy(nuevo->jugador.nombre, nombre);
    nuevo->jugador.disparos = disparos;
    nuevo->jugador.fecha = fecha;
    nuevo->siguiente = NULL;

    /*
        CASO 1:
        - la lista esta vacia, o
        - el nuevo jugador es mejor que el primero.

        En ambos casos el nuevo nodo pasa a ser el inicio de la lista.
    */
    if (*ranking == NULL ||
        disparos < (*ranking)->jugador.disparos)
    {
        nuevo->siguiente = *ranking;
        *ranking = nuevo;
    }
    else
    {
        /*
            CASO 2:
            El nuevo nodo va en el medio o al final.

            Se avanza mientras el SIGUIENTE jugador tenga una cantidad de
            disparos menor o igual a la del nuevo jugador.

            El <= hace que, si hay empate, los records que ya estaban en la
            lista queden antes que el nuevo.
        */
        actual = *ranking;

        while (actual->siguiente != NULL &&
               actual->siguiente->jugador.disparos <= disparos)
        {
            actual = actual->siguiente;
        }

        /*
            Se enlaza el nuevo nodo entre actual y el nodo que seguia.

            Antes:
                actual --------> siguiente

            Despues:
                actual -> nuevo -> siguiente
        */
        nuevo->siguiente = actual->siguiente;
        actual->siguiente = nuevo;
    }

    /* El TP5 pide conservar solamente los 5 mejores. */
    limitarRankingTop5(*ranking);

    return 1;
}

/*
    Deja solamente los primeros MAX_RANKING nodos de la lista.

    Como la lista ya esta ordenada, los primeros cinco son justamente los
    cinco mejores resultados.

    La estrategia es simple:
    1. avanzar hasta el quinto nodo;
    2. guardar la direccion de lo que viene despues;
    3. hacer que el quinto apunte a NULL;
    4. liberar todos los nodos sobrantes.

    Normalmente solo existira un sexto nodo, porque esta funcion se ejecuta
    despues de cada insercion. El while permite que tambien funcione si por
    algun motivo hubiera mas de uno.
*/
void limitarRankingTop5(struct nodo_ranking *ranking)
{
    struct nodo_ranking *actual;
    struct nodo_ranking *sobrante;
    struct nodo_ranking *auxiliar;
    int posicion;

    if (ranking == NULL)
    {
        return;
    }

    actual = ranking;
    posicion = 1;

    /* Se busca el quinto nodo. */
    while (actual->siguiente != NULL && posicion < MAX_RANKING)
    {
        actual = actual->siguiente;
        posicion++;
    }

    /*
        Si actual tiene un siguiente, significa que existen mas de cinco
        elementos. Todo lo que sigue queda fuera del Top 5.
    */
    if (posicion == MAX_RANKING && actual->siguiente != NULL)
    {
        sobrante = actual->siguiente;
        actual->siguiente = NULL;

        /* Se libera la memoria de los nodos descartados. */
        while (sobrante != NULL)
        {
            auxiliar = sobrante;
            sobrante = sobrante->siguiente;
            free(auxiliar);
        }
    }
}

/*
    Recorre la lista desde el primer nodo hasta NULL.

    A diferencia de un arreglo, no usamos un indice para acceder a cada
    elemento. Para avanzar hacemos:

        actual = actual->siguiente;
*/
void mostrarRanking(struct nodo_ranking *ranking)
{
    struct nodo_ranking *actual;
    struct tm *fecha;
    int posicion;

    if (ranking == NULL)
    {
        printf("\nRanking vacio.\n");
        return;
    }

    printf("\nTop 5 de jugadores:\n");

    actual = ranking;
    posicion = 1;

    while (actual != NULL)
    {
        fecha = localtime(&actual->jugador.fecha);

        if (fecha != NULL)
        {
            printf("%d. %s - %d disparos - "
                   "%02d/%02d/%04d %02d:%02d:%02d\n",
                   posicion,
                   actual->jugador.nombre,
                   actual->jugador.disparos,
                   fecha->tm_mday,
                   fecha->tm_mon + 1,
                   fecha->tm_year + 1900,
                   fecha->tm_hour,
                   fecha->tm_min,
                   fecha->tm_sec);
        }
        else
        {
            printf("%d. %s - %d disparos\n",
                   posicion,
                   actual->jugador.nombre,
                   actual->jugador.disparos);
        }

        actual = actual->siguiente;
        posicion++;
    }
}

/*
    Reconstruye la lista enlazada leyendo ranking.txt.

    Formato esperado de cada linea:

        nombre;disparos;AAAA-MM-DD HH:MM:SS

    Por cada linea valida se llama a insertarRanking(). Esa funcion coloca el
    nodo en su posicion correcta y mantiene automaticamente el Top 5.

    Esto tambien permite abrir un ranking.txt viejo del TP4 con mas de cinco
    resultados: en memoria quedaran solamente los cinco mejores.
*/
void cargarRanking(struct nodo_ranking **ranking)
{
    FILE *archivo;
    char linea[150];
    char nombre[MAX_NOMBRE];
    int disparos;
    int anio, mes, dia;
    int hora, minuto, segundo;
    struct tm datosFecha;
    time_t fecha;

    archivo = fopen(ARCHIVO_RANKING, "r");

    if (archivo == NULL)
    {
        /* Primera ejecucion: todavia puede no existir ranking.txt. */
        return;
    }

    while (fgets(linea, sizeof(linea), archivo) != NULL)
    {
        if (sscanf(linea,
                   "%49[^;];%d;%d-%d-%d %d:%d:%d",
                   nombre,
                   &disparos,
                   &anio, &mes, &dia,
                   &hora, &minuto, &segundo) == 8)
        {
            memset(&datosFecha, 0, sizeof(struct tm));

            datosFecha.tm_year = anio - 1900;
            datosFecha.tm_mon = mes - 1;
            datosFecha.tm_mday = dia;
            datosFecha.tm_hour = hora;
            datosFecha.tm_min = minuto;
            datosFecha.tm_sec = segundo;
            datosFecha.tm_isdst = -1;

            fecha = mktime(&datosFecha);

            if (fecha != (time_t) -1 && disparos > 0)
            {
                if (!insertarRanking(ranking,
                                     nombre,
                                     disparos,
                                     fecha))
                {
                    printf("\nNo se pudo reservar memoria para cargar el ranking.\n");
                    break;
                }
            }
        }
    }

    fclose(archivo);
}

/*
    Guarda el ranking recorriendo la lista nodo por nodo.

    Se usa modo "w" en vez de "a" porque el TP5 exige que ranking.txt refleje
    solamente los jugadores que quedaron en la lista Top 5.

    Si se siguiera usando "a", los jugadores descartados continuarian en el
    archivo y reaparecerian en futuras ejecuciones.
*/
int guardarRanking(struct nodo_ranking *ranking)
{
    FILE *archivo;
    struct nodo_ranking *actual;
    struct tm *datosFecha;

    archivo = fopen(ARCHIVO_RANKING, "w");

    if (archivo == NULL)
    {
        return 0;
    }

    actual = ranking;

    while (actual != NULL)
    {
        datosFecha = localtime(&actual->jugador.fecha);

        if (datosFecha != NULL)
        {
            fprintf(archivo,
                    "%s;%d;%04d-%02d-%02d %02d:%02d:%02d\n",
                    actual->jugador.nombre,
                    actual->jugador.disparos,
                    datosFecha->tm_year + 1900,
                    datosFecha->tm_mon + 1,
                    datosFecha->tm_mday,
                    datosFecha->tm_hour,
                    datosFecha->tm_min,
                    datosFecha->tm_sec);
        }

        actual = actual->siguiente;
    }

    fclose(archivo);
    return 1;
}

/*
    Muestra el mejor jugador.

    Como insertarRanking mantiene la lista ordenada, no hace falta buscar el
    minimo: el mejor jugador siempre esta en el primer nodo.
*/
void mostrarMejorJugador(struct nodo_ranking *ranking)
{
    time_t ahora;
    double diferencia;
    long minutosTotales;
    long dias;
    long horas;
    long minutos;

    if (ranking == NULL)
    {
        printf("\nTodavia no hay un record guardado.\n");
        return;
    }

    ahora = time(NULL);
    diferencia = difftime(ahora, ranking->jugador.fecha);

    if (diferencia < 0)
    {
        diferencia = 0;
    }

    minutosTotales = (long) (diferencia / 60.0);
    dias = minutosTotales / (24 * 60);
    horas = (minutosTotales % (24 * 60)) / 60;
    minutos = minutosTotales % 60;

    printf("\nRecord actual: %s con %d disparos.\n",
           ranking->jugador.nombre,
           ranking->jugador.disparos);

    printf("El record no se supera desde hace ");

    if (dias > 0)
    {
        printf("%ld dias, %ld horas y %ld minutos.\n",
               dias, horas, minutos);
    }
    else if (horas > 0)
    {
        printf("%ld horas y %ld minutos.\n",
               horas, minutos);
    }
    else
    {
        printf("%ld minutos.\n", minutos);
    }
}

/*
    Libera TODA la memoria dinamica usada por la lista.

    No alcanza con hacer free(ranking), porque cada nodo fue reservado con un
    malloc diferente. Hay que recorrerlos uno por uno.

    Se usa un doble puntero para que al terminar tambien podamos dejar la
    variable original del main en NULL.
*/
void liberarRanking(struct nodo_ranking **ranking)
{
    struct nodo_ranking *auxiliar;

    while (*ranking != NULL)
    {
        /* Guardamos el nodo que vamos a liberar. */
        auxiliar = *ranking;

        /* El inicio avanza al siguiente nodo ANTES del free. */
        *ranking = (*ranking)->siguiente;

        /* Se libera el nodo anterior. */
        free(auxiliar);
    }
}

/* ======================= HISTORIAL EN ARCHIVO ======================= */

/*
    Construye el nombre disparos_<nombre>.txt.

    Se reemplazan espacios y caracteres que pueden ser invalidos en nombres de
    archivo por un guion bajo. El nombre mostrado en el ranking no se modifica.
*/
void crearNombreArchivoDisparos(char nombre[], char nombreArchivo[])
{
    int i;
    int posicion;
    char caracter;

    strcpy(nombreArchivo, "disparos_");
    posicion = (int) strlen(nombreArchivo);

    for (i = 0; nombre[i] != '\0' && posicion < 110; i++)
    {
        caracter = nombre[i];

        if (caracter == ' ' || caracter == '/' || caracter == '\\' ||
            caracter == ':' || caracter == '*' || caracter == '?' ||
            caracter == '"' || caracter == '<' || caracter == '>' ||
            caracter == '|')
        {
            nombreArchivo[posicion] = '_';
        }
        else
        {
            nombreArchivo[posicion] = caracter;
        }

        posicion++;
    }

    nombreArchivo[posicion] = '\0';
    strcat(nombreArchivo, ".txt");
}

/*
    Guarda todos los disparos del jugador.

    Se abre el archivo en modo "a" para que, si una persona vuelve a jugar con
    el mismo nombre, sus nuevos disparos se agreguen al historial existente.
*/
int guardarHistorialJugador(char nombre[],
                            struct disparo *historial,
                            int cantidadDisparos)
{
    FILE *archivo;
    char nombreArchivo[130];
    int i;

    crearNombreArchivoDisparos(nombre, nombreArchivo);

    archivo = fopen(nombreArchivo, "a");

    if (archivo == NULL)
    {
        return 0;
    }

    for (i = 0; i < cantidadDisparos; i++)
    {
        fprintf(archivo, "%d,%d,%s\n",
                historial[i].fila,
                historial[i].columna,
                historial[i].resultado);
    }

    fclose(archivo);
    return 1;
}

/* ======================= GUARDAR Y CARGAR PARTIDA ======================= */

/*
    Guarda el estado completo de una partida en un archivo de texto.

    El archivo contiene, en este orden:
    - dimensiones del tablero;
    - posicion del barco;
    - cantidad de disparos;
    - estado de finalizacion;
    - contenido completo del tablero;
    - historial de disparos.
*/
int guardarPartida(char *tablero, int filas, int columnas,
                   int filaBarco, int colBarco,
                   int cantidadDisparos,
                   struct disparo *historial,
                   int barcoHundido)
{
    FILE *archivo;
    int i, j;

    archivo = fopen(ARCHIVO_PARTIDA, "w");

    if (archivo == NULL)
    {
        return 0;
    }

    fprintf(archivo, "%d %d\n", filas, columnas);
    fprintf(archivo, "%d %d\n", filaBarco, colBarco);
    fprintf(archivo, "%d\n", cantidadDisparos);
    fprintf(archivo, "%d\n", barcoHundido);

    /* Se guarda el tablero fila por fila. */
    for (i = 0; i < filas; i++)
    {
        for (j = 0; j < columnas; j++)
        {
            fprintf(archivo, "%c",
                    *(tablero + i * columnas + j));
        }
        fprintf(archivo, "\n");
    }

    /* Se guarda el historial para poder seguir agregando disparos al cargar. */
    for (i = 0; i < cantidadDisparos; i++)
    {
        fprintf(archivo, "%d %d %s\n",
                historial[i].fila,
                historial[i].columna,
                historial[i].resultado);
    }

    fclose(archivo);
    return 1;
}

/*
    Recupera una partida desde partida_guardada.txt.

    Primero carga todo en variables auxiliares. Solo cuando la lectura termina
    correctamente libera la partida anterior y entrega los nuevos datos a main.
    De esa forma, un archivo incompleto no destruye la partida que ya estaba en
    memoria.
*/
int cargarPartida(char **tablero,
                  int *filas, int *columnas,
                  int *filaBarco, int *colBarco,
                  int *cantidadDisparos,
                  struct disparo **historial,
                  int *barcoHundido)
{
    FILE *archivo;

    char *nuevoTablero = NULL;
    struct disparo *nuevoHistorial = NULL;

    int nuevasFilas;
    int nuevasColumnas;
    int nuevaFilaBarco;
    int nuevaColBarco;
    int nuevaCantidadDisparos;
    int nuevoBarcoHundido;

    int i, j;
    char celda;

    archivo = fopen(ARCHIVO_PARTIDA, "r");

    if (archivo == NULL)
    {
        return 0;
    }

    if (fscanf(archivo, "%d %d",
               &nuevasFilas, &nuevasColumnas) != 2 ||
        nuevasFilas <= 0 || nuevasColumnas <= 0)
    {
        fclose(archivo);
        return 0;
    }

    if (fscanf(archivo, "%d %d",
               &nuevaFilaBarco, &nuevaColBarco) != 2 ||
        nuevaFilaBarco < 0 || nuevaFilaBarco >= nuevasFilas ||
        nuevaColBarco < 0 || nuevaColBarco >= nuevasColumnas)
    {
        fclose(archivo);
        return 0;
    }

    if (fscanf(archivo, "%d", &nuevaCantidadDisparos) != 1 ||
        nuevaCantidadDisparos < 0)
    {
        fclose(archivo);
        return 0;
    }

    if (fscanf(archivo, "%d", &nuevoBarcoHundido) != 1 ||
        (nuevoBarcoHundido != 0 && nuevoBarcoHundido != 1))
    {
        fclose(archivo);
        return 0;
    }

    nuevoTablero = (char *) malloc(
        nuevasFilas * nuevasColumnas * sizeof(char));

    if (nuevoTablero == NULL)
    {
        fclose(archivo);
        return 0;
    }

    /*
        El espacio antes de %c hace que fscanf ignore saltos de linea y otros
        espacios antes de leer cada caracter del tablero.
    */
    for (i = 0; i < nuevasFilas; i++)
    {
        for (j = 0; j < nuevasColumnas; j++)
        {
            if (fscanf(archivo, " %c", &celda) != 1 ||
                (celda != '.' && celda != 'A' && celda != 'B'))
            {
                free(nuevoTablero);
                fclose(archivo);
                return 0;
            }

            *(nuevoTablero + i * nuevasColumnas + j) = celda;
        }
    }

    if (nuevaCantidadDisparos > 0)
    {
        nuevoHistorial = (struct disparo *) malloc(
            nuevaCantidadDisparos * sizeof(struct disparo));

        if (nuevoHistorial == NULL)
        {
            free(nuevoTablero);
            fclose(archivo);
            return 0;
        }

        for (i = 0; i < nuevaCantidadDisparos; i++)
        {
            if (fscanf(archivo, "%d %d %9s",
                       &nuevoHistorial[i].fila,
                       &nuevoHistorial[i].columna,
                       nuevoHistorial[i].resultado) != 3)
            {
                free(nuevoTablero);
                free(nuevoHistorial);
                fclose(archivo);
                return 0;
            }

            if (nuevoHistorial[i].fila < 0 ||
                nuevoHistorial[i].fila >= nuevasFilas ||
                nuevoHistorial[i].columna < 0 ||
                nuevoHistorial[i].columna >= nuevasColumnas ||
                (strcmp(nuevoHistorial[i].resultado, "agua") != 0 &&
                 strcmp(nuevoHistorial[i].resultado, "hundido") != 0))
            {
                free(nuevoTablero);
                free(nuevoHistorial);
                fclose(archivo);
                return 0;
            }
        }
    }

    fclose(archivo);

    /* La carga fue correcta: se reemplaza la partida anterior. */
    liberarMemoria(*tablero, *historial);

    *tablero = nuevoTablero;
    *historial = nuevoHistorial;

    *filas = nuevasFilas;
    *columnas = nuevasColumnas;
    *filaBarco = nuevaFilaBarco;
    *colBarco = nuevaColBarco;
    *cantidadDisparos = nuevaCantidadDisparos;
    *barcoHundido = nuevoBarcoHundido;

    return 1;
}

/* ======================= FUNCIONES AUXILIARES ======================= */

/*
    Libera la memoria de una partida anterior y reinicia sus estados antes de
    comenzar una partida nueva.
*/
void prepararNuevaPartida(char **tablero,
                          struct disparo **historial,
                          int *cantidadDisparos,
                          int *juegoIniciado,
                          int *barcoHundido)
{
    liberarMemoria(*tablero, *historial);

    *tablero = NULL;
    *historial = NULL;
    *cantidadDisparos = 0;
    *juegoIniciado = 0;
    *barcoHundido = 0;
}

/*
    Reemplaza los caracteres separadores usados por ranking.txt para evitar
    que el nombre altere el formato del archivo.
*/
void sanitizarNombre(char nombre[])
{
    int i;

    for (i = 0; nombre[i] != '\0'; i++)
    {
        if (nombre[i] == ';' || nombre[i] == ',')
        {
            nombre[i] = '_';
        }
    }
}

/*
    Libera toda la memoria dinamica de la partida.

    El ranking NO se libera aqui porque ahora tiene su propia funcion,
    liberarRanking(), que debe recorrer la lista nodo por nodo.
*/
void liberarMemoria(char *tablero, struct disparo *historial)
{
    if (tablero != NULL)
    {
        free(tablero);
    }

    if (historial != NULL)
    {
        free(historial);
    }
}

/*
    Descarta los caracteres pendientes de la entrada estandar hasta encontrar
    un salto de linea. Se usa al combinar scanf con fgets y al detectar una
    entrada no numerica.
*/
void limpiarBuffer(void)
{
    int caracter;

    while ((caracter = getchar()) != '\n' && caracter != EOF)
    {
    }
}
