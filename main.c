#include <stdio.h>
#include "restaurante.h"`r`n`r`nstatic void mostrarMenuPrincipal(void) {`r`n    printf("\n===== COSTOS DE RESTAURANTE =====\n");`r`n    printf("1. Gestion de ingredientes\n");`r`n    printf("2. Gestion de platos\n");`r`n    printf("3. Ingredientes por plato\n");`r`n    printf("4. Calculos y reportes\n");`r`n    printf("5. Guardar cambios\n");`r`n    printf("6. Salir\n");`r`n}`r`n`r`nint main(void) {
    Sistema sistema;
    int opcion;

    inicializarSistema(&sistema);
    cargarDatos(&sistema);

    do {
        mostrarMenuPrincipal();

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
