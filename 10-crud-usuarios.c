#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <errno.h>
#include <windows.h>

#define TIPO_STD "STD"
#define TIPO_ADMIN "ADMIN"
#define CAPACIDAD_TIPO sizeof(TIPO_ADMIN)
#define OPCION_TIPO_STD 1
#define OPCION_TIPO_ADMIN 2

typedef char cadena[200];

typedef struct {
    cadena mail;
    int pin;
    char tipo[CAPACIDAD_TIPO];
} Usuario;

#define MAX_USUARIOS 100
#define PIN_MINIMO 1000
#define PIN_MAXIMO 9999
#define NO_ENCONTRADO (-1)
#define SALIR 0
#define ALTA 1
#define LISTADO 2
#define CONSULTA 3
#define MODIFICACION 4
#define BAJA 5
#define REPORTE 6
#define VER_REPORTE 7
#define CARPETA_REPORTES "reportes"
#define ARCHIVO_REPORTE "usuarios-por-tipo.txt"
#define CAPACIDAD_RUTA (MAX_PATH + sizeof(CARPETA_REPORTES) + sizeof(ARCHIVO_REPORTE) + 2)

/* Descarta el resto de una linea para no contaminar la siguiente lectura. */
void descartarRestoDeLinea(void) {
    int caracter;
    do {
        caracter = getchar();
    } while (caracter != '\n' && caracter != EOF);
}

/* Funciones del catalogo utilizadas en este ejemplo. */
int leerEntero(cadena mensaje) {
    int numero;
    printf("%s: ", mensaje);
    scanf("%d", &numero);
    descartarRestoDeLinea();
    return numero;
}

int leerEnteroEntre(int valorMin, int valorMax, cadena mensaje) {
    int numero = leerEntero(mensaje);
    while (numero < valorMin || numero > valorMax) {
        printf("Error. Se esperaba un entero entre %d y %d.\n", valorMin, valorMax);
        numero = leerEntero(mensaje);
    }
    return numero;
}

void leerCadena(cadena mensaje, cadena target) {
    printf("%s: ", mensaje);
    fgets(target, sizeof(cadena), stdin);
    if (strchr(target, '\n') == NULL) {
        descartarRestoDeLinea();
    }
    target[strcspn(target, "\r\n")] = '\0';
}

void leerCadenaEntre(int minLength, int maxLength, cadena mensaje, cadena target) {
    leerCadena(mensaje, target);
    while (strlen(target) < (size_t) minLength || strlen(target) > (size_t) maxLength) {
        printf("Error. Se esperaba una cadena de entre %d y %d caracteres.\n",
               minLength, maxLength);
        leerCadena(mensaje, target);
    }
}
int esMailValido(const char mail[]) {
    const char *arroba = strchr(mail, '@');
    int valido = arroba != NULL;
    int i = 0;

    if (valido) {
        valido = arroba != mail && arroba[1] != '\0' &&
                 strchr(arroba + 1, '@') == NULL;
    }
    while (valido && mail[i] != '\0') {
        valido = !isspace((unsigned char) mail[i]);
        i++;
    }

    return valido;
}

void leerMail(cadena mail) {
    int valido;
    do {
        leerCadenaEntre(1, sizeof(cadena) - 1, "Mail", mail);
        valido = esMailValido(mail);
        if (!valido) {
            puts("Ingrese un mail con usuario y dominio, sin espacios.");
        }
    } while (!valido);
}
int buscarUsuario(const Usuario usuarios[], int cantidad, const char mail[]) {
    int posicion = NO_ENCONTRADO;
    int i = 0;

    while (i < cantidad && posicion == NO_ENCONTRADO) {
        if (strcmp(usuarios[i].mail, mail) == 0) {
            posicion = i;
        }
        i++;
    }

    return posicion;
}

void leerTipoUsuario(char tipo[]) {
    int opcion;
    printf("Tipo de usuario:\n%d. %s\n%d. %s\n",
           OPCION_TIPO_STD, TIPO_STD, OPCION_TIPO_ADMIN, TIPO_ADMIN);
    opcion = leerEnteroEntre(OPCION_TIPO_STD, OPCION_TIPO_ADMIN, "Opcion de tipo");
    switch (opcion) {
        case OPCION_TIPO_STD:
            strcpy(tipo, TIPO_STD);
            break;
        case OPCION_TIPO_ADMIN:
            strcpy(tipo, TIPO_ADMIN);
            break;
    }
}

void leerDatosUsuario(const Usuario usuarios[], int cantidad,
                      int posicionActual, Usuario *usuario) {
    int posicion;
    int mailDisponible;
    do {
        leerMail(usuario->mail);
        posicion = buscarUsuario(usuarios, cantidad, usuario->mail);
        mailDisponible = posicion == NO_ENCONTRADO || posicion == posicionActual;
        if (!mailDisponible) {
            puts("Ya existe un usuario con ese mail.");
        }
    } while (!mailDisponible);

    usuario->pin = leerEnteroEntre(PIN_MINIMO, PIN_MAXIMO, "PIN (1000 a 9999)");
    leerTipoUsuario(usuario->tipo);
}
void mostrarUsuario(const Usuario *usuario) {
    printf("Mail: %s | Tipo: %s\n", usuario->mail, usuario->tipo);
}

void listarUsuarios(const Usuario usuarios[], int cantidad) {
    int i;

    if (cantidad == 0) {
        puts("No hay usuarios cargados.");
    } else {
        for (i = 0; i < cantidad; i++) {
            mostrarUsuario(&usuarios[i]);
        }
    }
}

int solicitarPosicion(const Usuario usuarios[], int cantidad) {
    cadena mail;
    int posicion;
    leerMail(mail);
    posicion = buscarUsuario(usuarios, cantidad, mail);
    if (posicion == NO_ENCONTRADO) {
        puts("Usuario no encontrado.");
    }
    return posicion;
}

void agregarUsuario(Usuario usuarios[], int *cantidad) {
    Usuario nuevo;
    if (*cantidad == MAX_USUARIOS) {
        puts("Se alcanzo el limite de usuarios.");
    } else {
        leerDatosUsuario(usuarios, *cantidad, NO_ENCONTRADO, &nuevo);
        usuarios[*cantidad] = nuevo;
        (*cantidad)++;
        puts("Usuario creado.");
    }
}

void consultarUsuario(const Usuario usuarios[], int cantidad) {
    int posicion = solicitarPosicion(usuarios, cantidad);
    if (posicion != NO_ENCONTRADO) {
        mostrarUsuario(&usuarios[posicion]);
    }
}

void modificarUsuario(Usuario usuarios[], int cantidad) {
    Usuario modificado;
    int posicion = solicitarPosicion(usuarios, cantidad);
    if (posicion != NO_ENCONTRADO) {
        leerDatosUsuario(usuarios, cantidad, posicion, &modificado);
        usuarios[posicion] = modificado;
        puts("Usuario modificado.");
    }
}
void quitarUsuario(Usuario usuarios[], int *cantidad, int posicion) {
    int i;

    for (i = posicion; i < *cantidad - 1; i++) {
        usuarios[i] = usuarios[i + 1];
    }
    (*cantidad)--;
}

void eliminarUsuario(Usuario usuarios[], int *cantidad) {
    int posicion = solicitarPosicion(usuarios, *cantidad);
    if (posicion != NO_ENCONTRADO) {
        quitarUsuario(usuarios, cantidad, posicion);
        puts("Usuario eliminado.");
    }
}
int contarUsuariosPorTipo(const Usuario usuarios[], int cantidad, const char tipo[]) {
    int cantidadPorTipo = 0;
    int i;

    for (i = 0; i < cantidad; i++) {
        if (strcmp(usuarios[i].tipo, tipo) == 0) {
            cantidadPorTipo++;
        }
    }

    return cantidadPorTipo;
}

/* En Windows, obtiene la carpeta del .exe aunque se ejecute desde otra carpeta. */
int obtenerCarpetaReportes(char ruta[]) {
    char *ultimaBarra;
    DWORD longitud = GetModuleFileNameA(NULL, ruta, MAX_PATH);
    int obtenida = 0;

    if (longitud > 0 && longitud < MAX_PATH) {
        ultimaBarra = strrchr(ruta, '\\');
        if (ultimaBarra != NULL) {
            ultimaBarra[1] = '\0';
            strcat(ruta, CARPETA_REPORTES);
            obtenida = 1;
        }
    }

    return obtenida;
}

int prepararCarpetaReportes(char ruta[]) {
    int creada = 0;

    if (obtenerCarpetaReportes(ruta)) {
        /* Si la carpeta ya existe, tambien podemos usarla. */
        creada = _mkdir(ruta) == 0 || errno == EEXIST;
    }

    return creada;
}

int escribirReporteUsuarios(FILE *archivo, const Usuario usuarios[], int cantidad) {
    int cantidadStd = contarUsuariosPorTipo(usuarios, cantidad, TIPO_STD);
    int cantidadAdmin = contarUsuariosPorTipo(usuarios, cantidad, TIPO_ADMIN);
    int escrito;

    /* fprintf funciona como printf, pero escribe en el archivo indicado. */
    escrito = fprintf(archivo,
                      "REPORTE DE USUARIOS\n"
                      "===================\n"
                      "Total de usuarios: %d\n\n"
                      "Cantidad por tipo:\n"
                      "%s: %d\n"
                      "%s: %d\n",
                      cantidad, TIPO_STD, cantidadStd, TIPO_ADMIN, cantidadAdmin) >= 0;

    return escrito;
}

void generarReporteUsuarios(const Usuario usuarios[], int cantidad) {
    char ruta[CAPACIDAD_RUTA];
    FILE *archivo;
    int escrito;
    int cerrado;

    if (prepararCarpetaReportes(ruta)) {
        strcat(ruta, "\\" ARCHIVO_REPORTE);
        /* El modo "w" crea el archivo o reemplaza el reporte anterior. */
        archivo = fopen(ruta, "w");
        if (archivo != NULL) {
            escrito = escribirReporteUsuarios(archivo, usuarios, cantidad);
            cerrado = fclose(archivo) == 0;
            if (escrito && cerrado) {
                printf("Reporte generado en: %s\n", ruta);
            } else {
                puts("No se pudo completar la escritura del reporte.");
            }
        } else {
            puts("No se pudo abrir el archivo de reporte.");
        }
    } else {
        puts("No se pudo preparar la carpeta de reportes.");
    }
}

void verUltimoReporte(void) {
    char ruta[CAPACIDAD_RUTA];
    cadena linea;
    FILE *archivo;

    if (obtenerCarpetaReportes(ruta)) {
        strcat(ruta, "\\" ARCHIVO_REPORTE);
        /* El modo "r" abre un archivo existente para leerlo. */
        archivo = fopen(ruta, "r");
        if (archivo != NULL) {
            /* fgets devuelve NULL cuando ya no puede leer otra linea. */
            while (fgets(linea, sizeof linea, archivo) != NULL) {
                printf("%s", linea);
            }
            if (ferror(archivo)) {
                puts("No se pudo completar la lectura del reporte.");
            }
            fclose(archivo);
        } else {
            puts("No se pudo abrir el ultimo reporte. Genere uno con la opcion 6.");
        }
    } else {
        puts("No se pudo obtener la carpeta de reportes.");
    }
}

void mostrarMenu(void) {
    puts("\n--- Usuarios ---");
    printf("%d. Alta\n%d. Listado\n%d. Consulta\n%d. Modificacion\n"
           "%d. Baja\n%d. Generar reporte de usuarios\n%d. Ver ultimo reporte\n%d. Salir\n",
           ALTA, LISTADO, CONSULTA, MODIFICACION, BAJA, REPORTE, VER_REPORTE, SALIR);
}

int main(void) {
    Usuario usuarios[MAX_USUARIOS];
    int cantidad = 0;
    int opcion;

    do {
        mostrarMenu();
        opcion = leerEnteroEntre(SALIR, VER_REPORTE, "Opcion");
        switch (opcion) {
            case ALTA:
                agregarUsuario(usuarios, &cantidad);
                break;
            case LISTADO:
                listarUsuarios(usuarios, cantidad);
                break;
            case CONSULTA:
                consultarUsuario(usuarios, cantidad);
                break;
            case MODIFICACION:
                modificarUsuario(usuarios, cantidad);
                break;
            case BAJA:
                eliminarUsuario(usuarios, &cantidad);
                break;
            case REPORTE:
                generarReporteUsuarios(usuarios, cantidad);
                break;
            case VER_REPORTE:
                verUltimoReporte();
                break;
        }
    } while (opcion != SALIR);

    puts("Fin del programa.");
    return 0;
}
