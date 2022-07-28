#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 3 ;


//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}


//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

class Estanco : public HoareMonitor {
 private:
 static const int           // constantes:
   num_fumadores = 3;   //  núm. de fumadores e ingredientes
 int                        // variables permanentes
   mostrador ;          //  indica lo que hay en el mostrador
 CondVar                    
   mostrador_libre,                              //  cola en la que espera el productor
   obtencion_ingrediente[num_fumadores] ;                 //  cola en la que espera el fumador

 public:                       // constructor y métodos públicos
   Estanco(  ) ;           // constructor
   void  obtenerIngrediente( int valor );                
   void ponerIngrediente( int ingrediente  ); // el estanquero pone un ingrediente al azar
   void esperarRecogidaIngrediente();
};

Estanco::Estanco(){            //inicializar variables y tal
  mostrador = -1;
  mostrador_libre = newCondVar();

  for (int i=0; i<num_fumadores; i++){
      obtencion_ingrediente[i] = newCondVar();
  }

}
//----------------------------------------------------------------------
// función para obtener el ingrediente que utiliza el fumador para vacíar el mostrador
// y pillar su ingrediente.

void Estanco::obtenerIngrediente( int fumador ){
   //hasta que el ingrediente no sea el correspondiente del fumador, se bloquea
   if (mostrador != fumador){             
      obtencion_ingrediente[fumador].wait();
   }

   assert (0 <= mostrador && mostrador < num_fumadores);
   mostrador = -1;

   //notificamos que el mostrador está vacío
   mostrador_libre.signal();
}

//----------------------------------------------------------------------
// 

void Estanco::ponerIngrediente( int ingrediente  ){
   if (mostrador != -1){
      mostrador_libre.wait();
   }

   mostrador = ingrediente;

   //notificamos que el mostrador tiene el ingrediente producido
   obtencion_ingrediente[ingrediente].signal();
}


void Estanco::esperarRecogidaIngrediente(){
   if (mostrador != -1){
      mostrador_libre.wait();
   }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
   assert (0 <= num_fumador && num_fumador < num_fumadores);
   while (true){
      monitor->obtenerIngrediente(num_fumador);
      fumar(num_fumador);
   }
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
   int i;
   while(true){
      i = producir_ingrediente();
      monitor->ponerIngrediente(i);
      monitor->esperarRecogidaIngrediente();
   }
}


//----------------------------------------------------------------------

int main()
{
      cout << "-----------------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   //PARTE 0: DECLARACION DE HEBRAS
   thread estanquero; //HEBRA DEL ESTANQUERO
   thread fumadores[num_fumadores]; //VECTOR DE HEBRAS DE LOS FUMADORES
   MRef<Estanco> monitor = Create<Estanco>(); //MONITOR SU

   //PARTE 1: LANZAMIENTO DE LAS HEBRAS
   estanquero = thread(funcion_hebra_estanquero, monitor);
   
   for ( int i=0; i<num_fumadores; i++){
      fumadores[i] = thread(funcion_hebra_fumador,monitor,i);
   }
   
   //PARTE 2: SINCRONIZACION ENTRE LAS HEBRAS
   estanquero.join();

   for (int i=0; i<num_fumadores;i++){
      fumadores[i].join();
   }

   return 0;
}
