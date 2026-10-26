/* Informatica II - Parte B: registro de movimientos con sensores PIR.
 *
 * Recorrido de un evento:
 * 1. El PIR pasa de LOW a HIGH y dispara una interrupcion.
 * 2. La interrupcion guarda rapidamente la zona y el valor de millis()
 *    en una cola de espera.
 * 3. loop() retira esa captura y crea un nodo de la lista enlazada.
 * 4. Cada minuto se imprimen los nodos y se libera su memoria.
 *
 * Para B1, cambiar CANTIDAD_ZONAS a 1; para B2, dejarla en 2.
 * En Arduino Uno o Nano ATmega328P: zona 1 en D2 y zona 2 en D3.
 * Solo se registra el paso de LOW a HIGH: mantener HIGH no agrega eventos.
 * La fecha se ingresa por el Monitor Serie; no hace falta un reloj RTC.
 */
#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// uint8_t guarda numeros pequenos (como las zonas); uint32_t, milisegundos.
const uint8_t CANTIDAD_ZONAS = 2;
const uint32_t INTERVALO = 60000UL; // 60 000 ms = un minuto.

// Una interrupcion (ISR) debe terminar pronto. Por eso deja una "captura"
// en esta cola y delega el trabajo de reservar memoria a loop(). La cola
// es temporal; el registro que pide el TP es la lista enlazada de mas abajo.
struct Captura { uint32_t instante; uint8_t zona; };
const uint8_t TAM_COLA = 16; // Una posicion queda libre: caben 15 capturas.
// volatile indica que estos valores pueden cambiar fuera de loop(),
// mientras se ejecuta una interrupcion.
volatile Captura cola[TAM_COLA];
// entrada: proxima posicion para escribir; salida: proxima para leer.
volatile uint8_t entrada = 0, salida = 0;
volatile uint16_t perdidosCola = 0;

// Cada nodo representa un evento que se conserva hasta el informe.
struct Nodo {
  uint64_t fechaMs; // Milisegundos desde 01/01/2000 00:00:00.
  uint8_t zona;     // Sensor que detecto el movimiento: 1 o 2.
  Nodo *siguiente;  // Enlace al proximo evento; NULL si es el ultimo.
};
// primero y ultimo permiten recorrer la lista y agregar al final sin buscar.
Nodo *primero = NULL, *ultimo = NULL;
// cantidad cuenta los nodos actuales; los otros contadores registran perdidas.
uint16_t cantidad = 0, perdidosMemoria = 0;
uint64_t relojMs = 0; // Fecha y hora actuales expresadas en milisegundos.
uint32_t ultimoTick = 0, ultimoInforme = 0; // Referencias de millis().
bool iniciado = false; // Los PIR se habilitan despues de ingresar la fecha.
char linea[24];        // Espacio para la fecha recibida por Serie.
uint8_t usados = 0;    // Caracteres cargados en linea.
bool lineaLarga = false; // Detecta una entrada que no cabe en el arreglo.

// Esta funcion se llama desde una ISR: evita malloc, free, Serial y delay.
// millis() no avanza durante la ISR, pero una lectura breve sirve para
// marcar aproximadamente cuando comenzo la deteccion.
void capturar(uint8_t zona) {
  // Los indices vuelven a cero al llegar al final del arreglo: cola circular.
  uint8_t siguiente = (entrada + 1) % TAM_COLA;
  // Si la siguiente escritura alcanzaria a salida, la cola esta llena.
  if (siguiente == salida) {
    // Saturar el contador evita que vuelva a cero despues de 65535.
    if (perdidosCola < UINT16_MAX) ++perdidosCola;
    return;
  }
  cola[entrada].instante = millis();
  cola[entrada].zona = zona;
  entrada = siguiente; // Publicar solo cuando ambos datos estan completos.
}
// Cada pin necesita una funcion sin argumentos para attachInterrupt().
void interrupcionZona1() { capturar(1); }
void interrupcionZona2() { capturar(2); }

// Reglas del calendario: los siglos solo son bisiestos si son multiplos de 400.
bool bisiesto(uint16_t a) {
  return a % 4 == 0 && (a % 100 != 0 || a % 400 == 0);
}
// Devuelve la cantidad de dias del mes; febrero cambia en los bisiestos.
uint8_t diasMes(uint16_t a, uint8_t m) {
  const uint8_t dias[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  return dias[m - 1] + (m == 2 && bisiesto(a));
}

// Convierte el texto ingresado a milisegundos desde el 01/01/2000.
// Solo acepta AAAA-MM-DD HH:MM:SS, con un anio entre 2000 y 2099.
// resultado se pasa por referencia (&): la funcion puede escribir alli
// la fecha convertida y devolver, por separado, true o false.
bool interpretarFecha(const char *s, uint64_t &resultado) {
  // Primero se comprueban longitud, separadores y caracteres numericos.
  if (strlen(s) != 19) return false;
  for (uint8_t i = 0; i < 19; ++i) {
    if (i == 4 || i == 7) { if (s[i] != '-') return false; }
    else if (i == 10) { if (s[i] != ' ') return false; }
    else if (i == 13 || i == 16) { if (s[i] != ':') return false; }
    else if (s[i] < '0' || s[i] > '9') return false;
  }
  // Restar '0' convierte cada caracter, por ejemplo '7', en el numero 7.
  uint16_t a = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + s[3]-'0';
  uint8_t m = (s[5]-'0')*10+s[6]-'0', d = (s[8]-'0')*10+s[9]-'0';
  uint8_t h = (s[11]-'0')*10+s[12]-'0', min = (s[14]-'0')*10+s[15]-'0';
  uint8_t seg = (s[17]-'0')*10+s[18]-'0';
  // Rechazar horas fuera de rango y fechas imposibles, como 31 de abril.
  if (a < 2000 || a > 2099 || m < 1 || m > 12 || d < 1 ||
      d > diasMes(a,m) || h > 23 || min > 59 || seg > 59) return false;
  // Sumar los dias completos de los anios y meses anteriores.
  uint32_t dias = 0;
  for (uint16_t y = 2000; y < a; ++y) dias += bisiesto(y) ? 366 : 365;
  for (uint8_t mes = 1; mes < m; ++mes) dias += diasMes(a,mes);
  dias += d - 1; // El dia 1 empieza tras cero dias del mes actual.
  // Pasar dias, horas, minutos y segundos a una sola cifra en ms.
  // uint64_t evita desbordar los 32 bits durante esta conversion.
  resultado = (((uint64_t)dias * 24 + h)*60 + min)*60000 + seg*1000UL;
  return true;
}

// Agrega un cero delante de los numeros de 0 a 9 al imprimir la fecha.
void dosDigitos(uint8_t n) { if (n < 10) Serial.print('0'); Serial.print(n); }
// Realiza la operacion inversa: convierte los ms guardados en una fecha legible.
void imprimirFecha(uint64_t ms) {
  // Separar segundos completos y los tres digitos de milisegundos.
  uint32_t segundos = (uint32_t)(ms / 1000);
  uint16_t fraccion = ms % 1000;
  // El resto (%) extrae segundos, minutos y horas dentro de sus rangos.
  uint8_t seg = segundos % 60, min = (segundos / 60) % 60;
  uint8_t h = (segundos / 3600) % 24;
  uint32_t dias = segundos / 86400UL;
  // Descontar anios completos y luego meses hasta hallar la fecha exacta.
  uint16_t a = 2000;
  while (dias >= (uint32_t)(bisiesto(a) ? 366 : 365)) {
    dias -= bisiesto(a) ? 366 : 365; ++a;
  }
  uint8_t m = 1;
  while (dias >= diasMes(a,m)) { dias -= diasMes(a,m); ++m; }
  // dias quedo contado desde cero, por eso se suma uno al mostrarlo.
  Serial.print(a); Serial.print('-'); dosDigitos(m); Serial.print('-');
  dosDigitos(dias+1); Serial.print(' '); dosDigitos(h); Serial.print(':');
  dosDigitos(min); Serial.print(':'); dosDigitos(seg); Serial.print('.');
  if (fraccion < 100) Serial.print('0');
  if (fraccion < 10) Serial.print('0');
  Serial.print(fraccion);
}

// El reloj no tiene RTC: avanza sumando el tiempo transcurrido en millis().
// La resta entre valores sin signo sigue funcionando cuando millis() vuelve
// a cero (aproximadamente cada 49,7 dias), si loop() continua ejecutandose.
void actualizarReloj() {
  uint32_t ahora = millis();
  relojMs += (uint32_t)(ahora - ultimoTick);
  ultimoTick = ahora;
}

void agregarEvento(const Captura &c) {
  // Reservar un nodo nuevo en la RAM para conservar esta captura.
  Nodo *nuevo = (Nodo*)malloc(sizeof(Nodo));
  if (nuevo == NULL) { // Sin espacio: contar el evento perdido y seguir.
    if (perdidosMemoria < UINT16_MAX) ++perdidosMemoria;
    return;
  }
  actualizarReloj();
  // c.instante se tomo antes, en la interrupcion. Restar la demora entre
  // aquella captura y ultimoTick permite guardar la hora de deteccion,
  // aunque loop() haya tardado un poco en crear el nodo.
  nuevo->fechaMs = relojMs - (uint32_t)(ultimoTick - c.instante);
  nuevo->zona = c.zona;
  nuevo->siguiente = NULL;
  // Si la lista estaba vacia, este es tambien el primer nodo.
  // Si ya habia nodos, enlazarlo despues del ultimo.
  if (ultimo == NULL) primero = nuevo;
  else ultimo->siguiente = nuevo;
  ultimo = nuevo;
  ++cantidad;
}

void procesarCapturas() {
  // Tomar como limite la posicion de entrada actual: las capturas nuevas
  // quedaran para otra vuelta de loop() y no alargaran este recorrido.
  // Se detienen las interrupciones solo mientras se lee ese indice.
  noInterrupts();
  uint8_t limite = entrada;
  interrupts();
  while (salida != limite) {
    Captura c;
    // En AVR, copiar 32 bits requiere varias operaciones. Durante esta copia
    // breve se pausan las interrupciones para obtener una captura completa.
    // Despues se avanza salida y se habilitan de nuevo los sensores.
    noInterrupts();
    c.instante = cola[salida].instante;
    c.zona = cola[salida].zona;
    salida = (salida + 1) % TAM_COLA;
    interrupts();
    // malloc y el resto del trabajo se hacen fuera de la interrupcion.
    agregarEvento(c);
  }
}

// Una vez por minuto, mostrar todos los eventos y recuperar su RAM.
void informarYLiberar() {
  // F() conserva estos textos fijos en memoria de programa y ahorra SRAM.
  Serial.println(F("\n=== INFORME DEL MINUTO ==="));
  Serial.print(F("Eventos registrados: ")); Serial.println(cantidad);
  while (primero != NULL) {
    Nodo *actual = primero;
    Serial.print(F("Zona ")); Serial.print(actual->zona); Serial.print(F(" | "));
    imprimirFecha(actual->fechaMs); Serial.println();
    // Avanzar primero antes de liberar actual: despues de free() ya no
    // seria seguro leer ningun campo del nodo liberado.
    primero = actual->siguiente;
    free(actual);
  }
  ultimo = NULL; cantidad = 0; // La lista ya esta vacia para el proximo minuto.
  // perdidosCola cambia en la ISR y ocupa 16 bits: copiarlo y reiniciarlo
  // con las interrupciones pausadas evita una lectura o escritura incompleta.
  noInterrupts();
  uint16_t descartados = perdidosCola;
  perdidosCola = 0;
  interrupts();
  Serial.print(F("Perdidos por cola llena: ")); Serial.println(descartados);
  Serial.print(F("Perdidos por falta de memoria: ")); Serial.println(perdidosMemoria);
  // Ambos contadores vuelven a empezar para el siguiente informe.
  perdidosMemoria = 0;
  Serial.println(F("Lista liberada. Esperando el proximo minuto."));
}

// Leer caracteres del Monitor Serie hasta recibir una linea completa.
// El sistema espera aqui una fecha valida antes de habilitar los PIR.
void leerInicializacion() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue; // Compatible con LF y CR+LF.
    if (c != '\n') {
      // Dejar un byte libre para el terminador '\0'. Si sobra texto,
      // marcar toda la linea como invalida en vez de usarla truncada.
      if (usados < sizeof(linea)-1) linea[usados++] = c;
      else lineaLarga = true;
      continue;
    }
    linea[usados] = '\0'; // Convertir el arreglo en una cadena de C.
    uint64_t fecha;
    bool valida = !lineaLarga && interpretarFecha(linea,fecha);
    usados = 0; lineaLarga = false; // Prepararse para otra linea si hace falta.
    if (!valida) {
      Serial.println(F("Fecha invalida. Use AAAA-MM-DD HH:MM:SS (2000..2099)."));
      continue;
    }
    // Fijar la hora inicial y empezar a contar el minuto desde ahora.
    relojMs = fecha; ultimoTick = millis(); ultimoInforme = ultimoTick;
    iniciado = true;
    Serial.print(F("Sistema armado desde ")); imprimirFecha(relojMs); Serial.println();
    Serial.println(F("Active los PIR. El primer informe aparece en 60 segundos."));
    // RISING significa que se captura solo el cambio de LOW a HIGH.
    // digitalPinToInterrupt traduce D2/D3 a sus interrupciones externas.
    attachInterrupt(digitalPinToInterrupt(2),interrupcionZona1,RISING);
    if (CANTIDAD_ZONAS == 2)
      attachInterrupt(digitalPinToInterrupt(3),interrupcionZona2,RISING);
    return;
  }
}

// setup() corre una sola vez al encender o reiniciar la placa.
void setup() {
  Serial.begin(115200);
  // Los pines reciben la salida digital de cada sensor PIR.
  pinMode(2,INPUT); pinMode(3,INPUT);
  Serial.println(F("SEGURIDAD PIR - LISTA ENLAZADA"));
  Serial.println(F("Ingrese fecha y hora y pulse Enter: AAAA-MM-DD HH:MM:SS"));
  Serial.println(F("Ejemplo: 2026-10-24 17:00:00"));
  Serial.println(F("No se registran eventos hasta inicializar el reloj."));
}
// loop() se repite continuamente, sin delay(), para atender las capturas.
void loop() {
  // Hasta recibir la fecha, no se registran movimientos.
  if (!iniciado) { leerInicializacion(); return; }
  actualizarReloj();
  procesarCapturas();
  // Restar millis() como entero sin signo tambien funciona al desbordar.
  // Sumar INTERVALO al instante anterior mantiene el ritmo de informes.
  if ((uint32_t)(millis()-ultimoInforme) >= INTERVALO) {
    ultimoInforme += INTERVALO;
    informarYLiberar();
  }
}
