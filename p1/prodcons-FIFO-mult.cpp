#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 20 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   
   Semaphore puede_escribir = tam_vec; //declaración de semáforos
   Semaphore puede_leer= 0 ;
   int primera_libre = 0;              //posicion del primer componente libre del vector
   int primera_ocupada = 0;            //posicion del primer componente ocupado del vector
   int vector_intermedio[num_items];         //vector sobre el que se producen y consumen los datos

   int num_cons = 10; //numero de consumidores
   int num_prod = 5; //numero de productores

   mutex CON, PRO, LE;   //cerrojos


//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   //cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   //cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora( int h  )
{
   double ini = num_items/num_prod * h;         //delimitamos un intervalo de acción para cada hebra productora

   double fin = num_items/num_prod * (h+1);

   for( unsigned i = ini ; i < fin ; i++ )
   {
      int dato = producir_dato() ;
      //completado por mí
      sem_wait(puede_escribir);           //esperamos a que se pueda escribir
      PRO.lock();                     //entramos en sección crítica
      vector_intermedio[primera_libre] = dato;            //tocamos el vector compartido
      primera_libre++%tam_vec;                              //tocamos variable compartida
      PRO.unlock();                   //salimos de sección crítica
      sem_signal(puede_leer);             //tras producir, un dato más puede leerse, por lo que signal

      cout << endl << flush << "La hebra productora " << h << " ha producido " << dato << endl << flush;
   }     
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int h  )
{
   double ini = num_items/num_cons * h;

   double fin = num_items/num_cons * (h+1);

   for( unsigned i = ini ; i < fin ; i++ )
   {
      int dato ;
      //completado por mí desde aquí   
      sem_wait(puede_leer);            //esperamos a que se pueda leer
      CON.lock();                  //entramos en sección crítica
      dato = vector_intermedio[primera_ocupada];       //tocamos el vector compartido
      primera_ocupada++%tam_vec;                           //tocamos la variable compartida
      CON.unlock();                //salimos de sección crítica
      sem_signal(puede_escribir);      //tras consumir, podemos escribir otro dato en la posición libre
      //hasta aquí
      consumir_dato( dato ) ;

      cout << endl << flush << "La hebra consumidora " << h << " ha consumido " << dato << endl << flush;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores FIFO" << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

	thread consumidores[num_cons];
	
	thread productores[num_prod];
	
	for(int i = 0 ; i < num_prod ; i++)
		productores[i] = thread(funcion_hebra_productora, i);
	
	for(int j = 0 ; j < num_cons ; j++)
		consumidores[j] = thread(funcion_hebra_consumidora, j);
		
	for(int k = 0 ; k < num_prod ; k++)
		productores[k].join();
		
	for(int l = 0 ; l < num_cons ; l++)
		consumidores[l].join();

   test_contadores();
}