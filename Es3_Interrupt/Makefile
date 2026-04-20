# --- Configurazioni di base ---
MCU = 33EP512MU810
CC = xc16-gcc
BIN2HEX = xc16-bin2hex

# Nome dell'eseguibile finale
TARGET = Es3_Interrupt

# --- File sorgenti ---
# Compila di default tutti i file con estensione .c nella cartella di progetto corrente
SRCS = $(wildcard *.c)

# Togli il commento alla riga sottostante se il progetto usa la libreria custom ES
SRCS += ../ES/es_lib.c

# --- File oggetto ---
OBJS = $(SRCS:.c=.o)

# --- Flag di compilazione (C) ---
# -mcpu: specifica il microcontrollore target
# -I.: include file header della cartella corrente
# -I../ES: include i file header della libreria custom
CFLAGS = -mcpu=$(MCU) -I. -I../ES -O1 -g -Wall

# --- Flag di linking ---
# Il linker script p33EP512MU810.gld è localizzato automaticamente dal toolchain se l'ambiente è configurato correttamente
LDFLAGS = -mcpu=$(MCU) -Wl,--defsym=__MPLAB_BUILD=1

# --- Regole del Makefile ---

# Definisce il comportamento predefinito
all: $(TARGET).hex

# Regola per la generazione .hex (richiesta per il flash con IPE o Pickit)
$(TARGET).hex: $(TARGET).elf
	$(BIN2HEX) $<

# La generazione del file eseguibile .elf richiede gli oggetti .o compilati
$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

# Regola implicita di pattern per fare il build del .c e generare un file .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regola per le pulizie (da eseguire tramite 'make clean')
clean:
	rm -f *.o *.elf *.hex
