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
    
      chdir(args[1]) ;
      continue ; // Le decimos al Shell que vuelva a pedir otro comando (vete a la siguiente iteración del bucle infinito)
    }

    // Si se introduce el comando logout, que se salga del Shell
    // Salimos con 0 porque no estamos saliendo como un error (exit al programa principal, su padre es el terminal de linux)

    if(strcmp(args[0],"logout") == 0) exit(0) ; 
  
    // -------------------- //
    // 1º FASE (25/03/2023) //
    // -------------------- //

    pid_fork = fork() ; // Creamos un proceso hijo

    // El hijo va a tener en pid_fork un 0
    // El padre va a tener en pid_fork el PID de su hijo

    // Comienza la concurrencia
    if(pid_fork == 0) { // El hijo

      execvp(args[0],args) ; // Ejecución del comando introducido en el prompt

      // Si hay un error en el comando introducido, execvp no hace nada. (La ejecución salta a la siguiente línea)

      printf("Error. Comando %s no encontrado \n", args[0]) ;
      exit(-1) ; // El hijo sale con un error.
    
    } else { // El padre

      if(background == 0) { // Comando en primer plano

        pid_wait = waitpid(pid_fork,&status,0) ; // El padre espera al hijo porque proceso primer plano, va a devolver el PID que ha recogido
        status_res = analyze_status(status,&info) ; // Estado del proceso
        
        if(info != 255){ // Si el comando que se ha introducido es erróneo (info == 255) no printee esto.
          printf("\nComando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d. \n",
          args[0],pid_fork,status_strings[status_res],info) ;
        }

      } else { // Comando en segundo plano
        
        printf("\nComando %s ejecutado en segundo plano con pid %d. \n",
        args[0],pid_fork) ;
        // No es necesario poner la instrucción continue porque automaticamente al salirse de estos if else, se vá
        // a la siguiente iteración
      }
    }
  } // end while
} // end main()