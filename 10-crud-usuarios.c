#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
void mostrarMenu(void) {
    puts("\n--- Usuarios ---");
    printf("%d. Alta\n%d. Listado\n%d. Consulta\n%d. Modificacion\n"
           "%d. Baja\n%d. Salir\n", ALTA, LISTADO, CONSULTA,
           MODIFICACION, BAJA, SALIR);
}

int main(void) {
    Usuario usuarios[MAX_USUARIOS];
    int cantidad = 0;
    int opcion;

    do {
        mostrarMenu();
        opcion = leerEnteroEntre(SALIR, BAJA, "Opcion");
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
        }
    } while (opcion != SALIR);

    puts("Fin del programa.");
    return 0;
}