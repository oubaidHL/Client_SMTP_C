# /* auto-compilation et exécution via sourceage bash :)
gcc -Wall $0 -o $(basename $0 .c) && ./$(basename $0 .c)
exit $?
*/

/* Exemple d'une machine à état / d'un automate décrivant
 * un protocole, avec implémentation en C
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* soit l'automate suivant:
 *
 *          START <---\
 *            |       |
 *          QUERY_1   |
 *            |
 *          QUERY_2   |
 *            |       |
 *          COMPUTE   |
 *            |       |
 *            \--------
 *
 * avec l'information additionnelle:
 *    - toute entrée "q" à n'importe quel état fait quitter le programme
 *      sans erreur
 *    - toute entrée "s" à n'importe quel état fait repasser l'automate
 *      en état START
 */

typedef enum { START, QUERY_1, QUERY_2, COMPUTE } state_t;
#define LAST_STATE COMPUTE

#define MAX_LINE_LENGTH 100

int main(int argc, char **argv) {
   state_t state = START;
   int A = 0;
   int B = 0;

   while (1) {
      char buffer[MAX_LINE_LENGTH];      

      switch (state) {
	 case START:
            A = 0; B = 0;
	    printf("Bonjour, tapez une ligne vide ou q pour quitter, "
                   "s pour recommencer\n");
	    break;

	 case QUERY_1:
            printf("Entrez la valeur A, q pour quitter, s pour recommencer\n");
	    break;

	 case QUERY_2:
            printf("Entrez la valeur B, q pour quitter, s pour recommencer\n");
	    break;

	 case COMPUTE:
            printf("Le résultat vaut: %d. Faites q pour quitter, ou tout autre entrée pour recommencer\n",
                   A + B);
	    break;

	 default:
	    fprintf(stderr,
		    "%s: illegal state %d; aborted\n",
		    argv[0],
		    state);
	    return EXIT_FAILURE;
      }

      if (fgets(buffer, sizeof(buffer), stdin)) {
         if (strlen(buffer) && (buffer[strlen(buffer) - 1] == '\n')) {
            /* supprimer la fin de ligne */
            buffer[strlen(buffer) - 1] = '\0';
         }

         if (!strcmp("q", buffer)) {
            fprintf(stderr, "%s: FIN AUTOMATE\n", argv[0]);
            return EXIT_SUCCESS;
         }
         else if (!strcmp("s", buffer)) {
            printf("%s: RESTART AUTOMATE\n", argv[0]);
            state = START;
         }
         else {
            if (state == QUERY_1) {
               A = atoi(buffer);
            }
            else if (state == QUERY_2) {
               B = atoi(buffer);
            }

            state++; /* suppose ordonnés */
            if (state == (LAST_STATE + 1)) {
               state = START;
            }                                    
         }
      }
      else {
         /* on quitte aussi en cas d'EOF */
         fprintf(stderr, "%s: EOF\n", argv[0]);
         return EXIT_FAILURE;
      }
   }

   /* NOT REACHED */   
}
