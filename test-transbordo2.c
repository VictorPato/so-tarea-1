#include <nSystem.h>
#include "transbordo.h"

int debugging= TRUE;
int verificar, achacao, apargua;

typedef struct {
  int i; /* transbordador */
  int v; /* vehiculo */
  nTask t;
  int haciaChacao;
} Viaje;

/* Guarda el identificador de la tarea nMain que sirve para controlar
   el avance del resto de las tareas */
nTask ctrl;

/* Procedimientos para los tests */

Viaje *esperarTransbordo();
void continuarTransbordo(Viaje *);

int testUnTransbordo(int (*tipo)(), int v);
int testUnTransbordoVacio(int (*tipo)(), int v, int haciaChacao);

int norteno(int v), nortenoConMsg(int v);
int isleno(int v), islenoConMsg(int v);

int automovilista(int v, int n);

int nMain( int argc, char **argv ) {
  ctrl= nCurrentTask();
  inicializar(1000);
  verificar= TRUE;
  int autos = 50000;
  int vueltas = 10;
  nSetTimeSlice(1);

  nPrintf("Nuevo test: Una prueba con muchos autos y transbordadores.\n");
  nPrintf("%d Autos van a tratar de pasar %d veces ida y vuelta.\n",autos,vueltas);
  nPrintf("No debe suceder deadlock.\n");
  nTask tasks[autos];
  verificar= FALSE;
  for (int t=0; t<autos; t++) {
    tasks[t]= nEmitTask(automovilista, t, vueltas);
  }
  for (int t=0; t<autos; t++) {
    nWaitTask(tasks[t]);
  }

  nPrintf("El test termino correctamente! Yajuu!\n\n");
  finalizar();
  return 0;
}


int testUnTransbordo(int (*tipo)(), int v) {
  // Precondicion: hay transbordadores disponibles en la misma orilla
  // del vehiculo v.
  // Embarca, transborda y desembarca vehiculo v.  Retorna el transbordador
  // usado que queda estacionado en la otra orilla.
  nTask vehiculoTask= nEmitTask(tipo, v); /* vehiculo v */
  Viaje *viaje= esperarTransbordo();
  int i= viaje->i; /* el transbordador usado */
  if (viaje->v!=v)
    nFatalError("testUnTransbordo", "Se transborda el vehiculo incorrecto\n");
  if ( !(0<=i && i<3) )
    nFatalError("testUnTransbordo", "El trabordador debe estar entre 0 y 2\n");
  continuarTransbordo(viaje);
  nWaitTask(vehiculoTask);
  return i;
}

int testUnTransbordoVacio(int (*tipo)(), int v, int haciaChacao) {
  // Precondicion: *no* hay transbordadores disponibles en la misma orilla
  // del vehiculo v.
  // Embarca, transborda y desembarca vehiculo v.  Retorna el transbordador
  // usado que queda estacionado en la otra orilla.
  // Verifica que un transbordador hizo un viaje vacio.
  nTask t;
  nTask vehiculoTask= nEmitTask(tipo, v); /* vehiculo v */
  Viaje *viaje= esperarTransbordo(); /* Este viaje no lleva auto */
  int i= viaje->i, old=i; /* el transbordador usado */
  if (viaje->v>=0)
    nFatalError("testUnTransbordoVacio",
                "No se debio transportar ningun vehiculo\n");
  if (viaje->haciaChacao==haciaChacao)
    nFatalError("testUnTransbordoVacio",
                "Este viaje es en la direccion incorrecta\n");
  continuarTransbordo(viaje);
  viaje= esperarTransbordo(); /* Este viaje si que lleva a v */
  if (i!=old)
    nFatalError("testUnTransbordo", "Se debio usar el mismo transbordador\n");
  if (viaje->v!=v)
    nFatalError("testUnTransbordo", "Se transborda el vehiculo incorrecto\n");
  if ( !(0<=i && i<3) )
    nFatalError("testUnTransbordo", "El trabordador debe estar entre 0 y 2\n");
  if (viaje->haciaChacao!=haciaChacao)
    nFatalError("testUnTransbordoVacio",
                "Este viaje es en la direccion incorrecta\n");
  if (nReceive(NULL, 1)!=NULL)
    nFatalError("testUnTransbordoVacio",
                "Este mensaje no debio haber llegado\n");
  continuarTransbordo(viaje);
  /* Ahora deberia llegar el mensaje falso */
  viaje= nReceive(&t, -1);
  if (viaje->v!= 1000)
    nFatalError("testUnTransbordoVacio",
                "Debio haber llegado un mensaje falso\n");
  nReply(t, 0);
  nWaitTask(vehiculoTask);
  return i;
}

int norteno(int v) {
  transbordoAChacao(v);
  return 0;
}

int nortenoConMsg(int v) {
  Viaje falso;
  falso.v= 1000;
  transbordoAChacao(v);
  /* Si transbordoAChacao retorna antes de invocar haciaChacao, este
     mensaje va hacer fallar los tests */
  return nSend(ctrl, &falso);
}

int isleno(int v) {
  transbordoAPargua(v);
  return 0;
}

int islenoConMsg(int v) {
  Viaje falso;
  falso.v= 1000;
  transbordoAPargua(v);
  // Si transbordoAPargua retorna antes de invocar haciaPargua, este
  // mensaje va hacer fallar los tests
  return nSend(ctrl, &falso);
}

// haciaChacao: Ud. invoca esta funcion en transbordo.c cuando se invoca
// transbordoAChacao.
void haciaChacao(int i, int v) {
  if (!verificar)
    achacao++;
  else {
    Viaje viaje;
    viaje.i= i;
    viaje.v= v;
    viaje.haciaChacao= TRUE;
    // Notifica al nMain que se invoco haciaChacao.  El vehiculo v va
    // hacia Chacao.  Llegara cuando nMain invoque continuarTransbordo(&viaje).
    nSend(ctrl, &viaje);
    // Ahora si se desembarco v.  La funcion retorna con lo que
    // transbordoAChacao puede retornar.
  }
}


// haciaPargua: Ud. invoca esta funcion en transbordo.c cuando se invoca
// transbordoAPargua.
void haciaPargua(int i, int v) {
  if (!verificar)
    apargua++;  // Solo valido para el test de esfuerzo
  else {
    Viaje viaje;
    viaje.i= i;
    viaje.v= v;
    viaje.haciaChacao= FALSE;
    // Notifica al nMain que se invoco haciaPargua.  El vehiculo v va
    // hacia Pargua.  Llegara cuando nMain invoque continuarTransbordo(&viaje).
    nSend(ctrl, &viaje);
    // Termino el transbordo.  La funcion retorna con lo que
    // transbordoAPargua puede retornar.
  }
}

Viaje *esperarTransbordo() {
  nTask t;
  // Espera la notificacion del inicio de cualquier transbordo llamando
  // haciaPargua o haciaChacao.
  // Retorna en viaje el transbordador usado y el vehiculo que lleva.
  Viaje *viaje= nReceive(&t, -1);
  viaje->t= t;
  return viaje;
}

void continuarTransbordo(Viaje *viaje) {
  // Invocada por nMain.
  // Hace que termine el transbordo de viaje->i con el vehiculo viaje->v.
  // Ese transbordador llego a la orilla y por lo tanto la
  // llamada a haciaPargua o haciaChacao retorna.  El transbordador queda
  // libre para otro viaje.
  nReply(viaje->t, 0);
}

int automovilista(int v, int n) {
  int k;
  for (k=0; k<n; k++) {
    transbordoAChacao(v);
    transbordoAPargua(v);
  }
  return 0;
}
