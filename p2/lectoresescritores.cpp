// g++ -std=c++11 -pthread -I. -o prodcons-mult lectoresescritores.cpp scd.cpp HoareMonitor.hpp 


#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

//Variables globales
const int num_lectores = 3,
num_escritores = 3;
mutex mtx;                    //cerrojo para las salidas por pantalla.

void escribir(int num_escritor){
   chrono::milliseconds duracion_escritura(aleatorio<20,200>());

   mtx.lock();
   cout << "Escritor" << num_escritor << " escribiendo... ("
        << duracion_escritura.count() << " milisegundos)" << endl;
   mtx.unlock();

   this_thread::sleep_for(duracion_escritura);
}

void leer(int numLector){
   chrono::milliseconds duracion_lectura(aleatorio<20,200>());

   mtx.lock();
   cout << "Lector " << numLector << "leyendo... ("
   << duracion_lectura.count() << " milisegundos)" << endl;
   mtx.unlock();

   this_thread::sleep_for(duracion_lectura);
}

class Lec_Esc : public HoareMonitor{

   //VARIABLES PERMANENTES
   int n_lec; //Numero de lectores
   bool escrib; //Comprueba que haya un escritor activo

   //VARIABLES DE CONDICION
   CondVar lectura,  //Cola de lectores
   escritura;   //Cola de escritores
   
  
   public:
      Lec_Esc(); //Constructor por defecto
      void ini_lectura(); //Función de inicio de lectura
      void fin_lectura(); //Función de fin de lectura
      void ini_escritura(); //Función de inicio de escritura
      void fin_escritura(); //Función de fin de escritura
};


//-----------------------------------------------------
//Constructor
Lec_Esc::Lec_Esc(){
   n_lec = 0;
   escrib = false;
   lectura = newCondVar();
   escritura = newCondVar();
}

//------------------------------------------------------------------------------------
// Inicio de lectura
void Lec_Esc::ini_lectura(){
   //Comprobar que no haya escritor
   if(escrib)
   lectura.wait(); //Esperamos a que la lectura sea posible (sin escritores)

   //Agregamos un lector más
   n_lec++;

   //Desbloqueamos a los lectores
   lectura.signal();
}

//------------------------------------------------------------------------------------
// Fin de lectura
void Lec_Esc::fin_lectura(){
   //El lector termina la lectura, por lo que decrementamos el contador
   n_lec--;
   //Si no quedan más lectores, desbloqueamos a los escritores
   if(n_lec == 0)
      escritura.signal();
}

//------------------------------------------------------------------------------------
// Inicio de escritura
void Lec_Esc::ini_escritura(){
   if((n_lec > 0) or escrib)
      escritura.wait();

   escrib = true;
 }


//------------------------------------------------------------------------------------
// Fin de escritura
void Lec_Esc::fin_escritura(){
   //Cambiar variable para indicar que ya no hay escritores
   escrib = false;
   //Si hay lectores, avisar a uno
   if(!lectura.empty())
      lectura.signal();
   //Si no, avisar a un escritor
   else
      escritura.signal();
}

//------------------------------------------------------------------------------------
// Funciones de las hebras

void funcion_hebra_lector(MRef<Lec_Esc> monitor, int numLector){
   while(true){
      //Retraso aleatorio
      chrono::milliseconds retraso(aleatorio<20,200>());
      this_thread::sleep_for(retraso);

      //Funcion
      monitor->ini_lectura();
      leer(numLector);
      monitor->fin_lectura();
   }
}

void funcion_hebra_escritor(MRef<Lec_Esc> monitor, int num_escritor){
   while(true){
      //Retraso aleatorio
      chrono::milliseconds retraso(aleatorio<20,200>());
      this_thread::sleep_for(retraso);

      //Funcion
      monitor->ini_escritura();
      escribir(num_escritor);
      monitor->fin_escritura();
   }
}

int main(){
   //PARTE 0: DECLARACION DE HEBRAS
   assert(0 < num_lectores && 0 < num_escritores);
   thread lectores[num_lectores];
   thread escritores[num_escritores];
   MRef<Lec_Esc> monitor = Create<Lec_Esc>();

   //PARTE 1: LANZAMIENTO DE LAS HEBRAS
   for(int i=0; i<num_lectores; i++){
      lectores[i] = thread(funcion_hebra_lector, monitor, i);
   }

   for(int i=0; i<num_escritores; i++){
      escritores[i] = thread(funcion_hebra_escritor, monitor, i);
   }

   //PARTE 2: SINCRONIZACION ENTRE LAS HEBRAS
   for(int i=0; i<num_lectores; i++){
      lectores[i].join();
   }

   for(int i=0; i<num_escritores; i++){
      escritores[i].join();
   }

   return 0;
}