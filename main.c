#include <stdio.h>
#include "restaurante.h"

int main(void) {
    Sistema sistema;
    int opcion;

    inicializarSistema(&sistema);
    cargarDatos(&sistema);

    do {
        printf("\n===== COSTOS DE RESTAURANTE =====\n");
        printf("1. Gestion de ingredientes\n");
        printf("2. Gestion de platos\n");
        printf("3. Ingredientes por plato\n");
        printf("4. Calculos y reportes\n");
        printf("5. Guardar cambios\n");
        printf("6. Salir\n");

        opcion = leerEntero("Opcion: ", 1, 6);

        switch (opcion) {
            case 1:
                menuIngredientes(&sistema);
                break;
            case 2:
                menuPlatos(&sistema);
                break;
            case 3:
                menuRelaciones(&sistema);
                break;
            case 4:
                menuReportes(&sistema);
                break;
            case 5:
                guardarDatos(&sistema);
                printf("Cambios guardados.\n");
                pausar();
                break;
            case 6:
                if (confirmar("Desea guardar antes de salir")) {
                    guardarDatos(&sistema);
                }
                printf("Programa finalizado.\n");
                break;
        }
    } while (opcion != 6);

    return 0;
}
