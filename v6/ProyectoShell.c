/*------------------------------------------------------------------------------
Proyecto Shell de UNIX. Sistemas Operativos
Grados I. Informática, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Algunas secciones están inspiradas en ejercicios publicados en el libro
"Fundamentos de Sistemas Operativos", Silberschatz et al.

Para compilar este programa: gcc ProyectoShell.c ApoyoTareas.c -o MiShell
Para ejecutar este programa: ./MiShell
Para salir del programa en ejecución, pulsar Control+D
------------------------------------------------------------------------------*/

#include "ApoyoTareas.h" // Cabecera del módulo de apoyo ApoyoTareas.c
 
#define MAX_LINE 256 // 256 caracteres por línea para cada comando es suficiente
#include <string.h>  // Para comparar cadenas de cars. (a partir de la tarea 2)


// Declaramos el puntero que apunta a la lista de tareas
// Se declara de forma global para que el método manejador pueda trabajar con él.

job * ptrLista ;


void manejador(int signal){ // Recibe el número de la señal (SEÑAL SIGCHILD = 17)

  block_SIGCHLD() ;

  job * item ;
  int status ;
  int info ;
  int pid_wait = 0 ;
  enum status status_res ;

  for(int i = 1 ; i <= list_size(ptrLista); i++){ // Recorro la lista (OJO: la lista empieza en 1)

    // En cada iteración hacemos un wait a cada uno de los procesos

    item = get_item_bypos(ptrLista,i) ;
    pid_wait = waitpid(item->pgid,&status, WUNTRACED | WNOHANG ) ; // Si recojo un proceso, guardo el proceso que he recogido

    if (pid_wait == item->pgid) { // Si esto se cumple, significa que he recogido algún proceso.

      status_res = analyze_status(status,&info) ;

      if(status_res == FINALIZADO || status_res == SENALIZADO) {

        printf("\nComando %s ejecutado en segundo plano con PID %d ha concluido\n",item->command,item->pgid) ;
        delete_job(ptrLista,item) ;

      } else if (status_res == SUSPENDIDO) {

        printf("\nComando %s ejecutado en segundo plano con PID %d ha suspendido su ejecución\n",item->command,item->pgid) ;
        item->ground = DETENIDO ;
      
      } else if (status_res == REANUDADO) {
        item->ground = SEGUNDOPLANO ;
      }
    }
  }
  unblock_SIGCHLD() ;
}

// --------------------------------------------
//                     MAIN          
// --------------------------------------------

int main(void)
{

  char inputBuffer[MAX_LINE]; // Búfer que alberga el comando introducido
  int background;         // Vale 1 si el comando introducido finaliza con '&'
  char *args[MAX_LINE/2]; // La línea de comandos (de 256 cars.) tiene 128 argumentos como máx
  
  // Variables de utilidad:
  int pid_fork, pid_wait; // pid para el proceso creado y esperado
  int status;             // Estado que devuelve la función wait
  enum status status_res; // Estado procesado por analyze_status()
  int info;		      // Información procesada por analyze_status()

  // Variables creadas por el alumno

  job * item ;
  int firstground = 0 ; // Inicializada a 0 porque no inicialmente no hay ningún proceso en primer plano cuando arrancamos el terminal

  ignore_terminal_signals() ; 


  /*
  SIGCHILD: es una señal que se lanza al padre cuando un proceso hijo termina o se suspende. El padre delega esta señal
  al manejador.
  MANEJADORES: es como un bloque try catch de java, es un subprograma cuya función es manejar una o varias señales.
  Va a recibir las señales delegadas por el padre del hijo que manda la señal. OJO: no sabemos que hijo ha mandado la señal.
  Hay que recorrer toda la lista de jobs haciendo un waitpid a cada hijo con el fin de recogerlo.
  En caso de que el proceso haya terminado, lo eliminamos. Si estaba suspendido pues actualizamos la información de la lista.

  */

  // Cuando llegue la señal SIGCHILD, usa este manejador para controlarla
  signal(SIGCHLD,manejador) ;

  // Creamos la lista de tareas

  ptrLista = new_list("listaTareas") ;

  while (1) // El programa termina cuando se pulsa Control+D dentro de get_command()
  {   		
    printf("COMANDO->");
    fflush(stdout);
    get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el próximo comando
    if (args[0]==NULL) continue; // Si se introduce un comando vac�o, no hacemos nada


    // -------------------- //
    // 2º FASE (25/03/2023) // (Los dos siguientes if)
    // -------------------- // 

    // Uso del comando cd: cd /Escritorio/Utilidades/Matlab 
    // args[0] = cd y args[1] = /Escritorio/Utilidades/Matlab
    
    if(strcmp(args[0],"cd") == 0) { // Si se introduce el comando cd, que se cambie el directorio
    
      int val = chdir(args[1]) ;

      if(val == 0) {
        continue ; // Le decimos al Shell que vuelva a pedir otro comando (vete a la siguiente iteración del bucle infinito)
      } else {
        printf("Directorio no reconocido \n") ;
      }
    }

    // Si se introduce el comando logout, que se salga del Shell
    // Salimos con 0 porque no estamos saliendo como un error (exit al programa principal, su padre es el terminal de linux)

    if(strcmp(args[0],"logout") == 0) exit(0) ; 

    // Comandos internos de la fase 5

    // -------------------- //
    // 5º FASE (12/04/2023) //
    // -------------------- //

    if(strcmp(args[0],"jobs") == 0) { 
      block_SIGCHLD() ;
      if(empty_list(ptrLista) != 1) { // Si la lista no esta vacía
        print_job_list(ptrLista) ;
      } else {
        printf("La lista esta vacía \n") ;
      }
      unblock_SIGCHLD() ;
      continue ; 
    }

    if(strcmp(args[0],"fg") == 0) { 
      block_SIGCHLD() ;
      int posicion = 1 ; // Si no te dan la posición, cojo el primer elemento de la lista
      firstground = 1 ;

      if(args[1] != NULL) { // Si te dan la posición de la lista

        posicion = atoi(args[1]) ; // Uso la función stoi para pasar de string a int
      }

      item = get_item_bypos(ptrLista,posicion) ;
      
      if(item != NULL) { // Recuerda que en la lista solo se guardan procesos en segundo plano o suspendidos

        set_terminal(item->pgid) ;

        if(item->ground == DETENIDO) {
          killpg(item->pgid,SIGCONT) ;
        }

        pid_fork = item->pgid ;

        delete_job(ptrLista,item) ;

        // No es necesario cambiar el ground del proceso a primer plano porque vamos a borrarlo de la lista de segundo plano o suspendidos

      } else {
        printf("Posición de lista errónea \n") ;
      }
      unblock_SIGCHLD() ; 
    }

    if(strcmp(args[0],"bg") == 0) { 
      block_SIGCHLD() ;
      int posicion = 1 ; // Si no te dan la posición, cojo el primer elemento de la lista
      

      if(args[1] != NULL) { // Si te dan la posición de la lista

        posicion = atoi(args[1]) ;
      }

      item = get_item_bypos(ptrLista,posicion) ;

      if((item != NULL) && (item->ground == DETENIDO)) {
        item->ground = SEGUNDOPLANO ;
        killpg(item->pgid,SIGCONT) ;

      } else {
        printf("Posición de lista errónea o el proceso no estaba en estado DETENIDO \n") ;
      }
      unblock_SIGCHLD() ;
      continue ;
    }


    // -------------------- //
    // 1º FASE (25/03/2023) //
    // -------------------- //

    if(firstground == 0) pid_fork = fork() ; // Creamos un proceso hijo si no hay un proceso en primer plano ya

    /*
      Si hay un proceso en primer plano, la variable pid_fork va a tener el pid del padre del proceso en primer plano.
    */

    // El hijo va a tener en pid_fork un 0
    // El padre va a tener en pid_fork el PID de su hijo

    // Comienza la concurrencia
    if(pid_fork == 0) { // El hijo

      new_process_group(getpid()) ; // El hijo crea su propio grupo de procesos. 

      // background = 1 -> Tarea en segundo plano
      // background = 0 -> Tarea en primer plano
      if(background == 0) {
      
        set_terminal(getpid()) ;
      }
      restore_terminal_signals() ;
      

      execvp(args[0],args) ; // Ejecución del comando introducido en el prompt

      // Si hay un error en el comando introducido, execvp no hace nada. (La ejecución salta a la siguiente línea)

      printf("Error. Comando %s no encontrado \n", args[0]) ;
      exit(-1) ; // El hijo sale con un error.
    
    } else { // El padre

      if(background == 0) { // Comando en primer plano

        pid_wait = waitpid(pid_fork,&status,WUNTRACED) ; // El padre espera al hijo porque proceso primer plano, va a devolver el PID que ha recogido
        
        set_terminal(getpid()) ; // Recuperamos el terminal cedido a nuestro hijo

        status_res = analyze_status(status,&info) ; // Estado del proceso
        
        // Si el comando que se ha introducido es erróneo (info == 255) no printee esto.
        // printea el mensaje deseado.

        if (info != 255){

          if(status_res == FINALIZADO) {
            printf("\nComando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d. \n",
            args[0],pid_fork,status_strings[status_res],info) ;
          }

          if(status_res == SUSPENDIDO) {
            printf("\nComando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d. \n",
            args[0],pid_fork,status_strings[status_res],info) ;

            block_SIGCHLD() ;

            // Insertar el comando en la lista de tareas
            item = new_job(pid_fork,args[0],DETENIDO) ;
            add_job(ptrLista,item) ;

            unblock_SIGCHLD() ;
          }
        }
        firstground = 0 ; // Ya no hay proceso en primer plano

      } else { // Comando en segundo plano
        
        printf("\nComando %s ejecutado en segundo plano con pid %d. \n",
        args[0],pid_fork) ;
        // No es necesario poner la instrucción continue porque automaticamente al salirse de estos if else, se vá
        // a la siguiente iteración

        block_SIGCHLD() ;

        // Insertar el comando en la lista de tareas
        item = new_job(pid_fork,args[0],SEGUNDOPLANO) ;
        add_job(ptrLista,item) ;

        unblock_SIGCHLD() ;
      }
    }
  } // end while
} // end main()