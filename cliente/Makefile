# ==============================================================================
# Makefile para o Cliente Logger (CORBA / TAO)
# Disciplina: Programação Distribuída / Computação Distribuída
# ==============================================================================

CXX = g++
CXXFLAGS = -Wall -O2 -std=c++11

# Diretórios do projeto
BIN_DIR = ../bin
IDL_DIR = ../idl

# Detecção e inclusão do ACE / TAO
INCLUDES = -I. -I$(IDL_DIR)

ifdef ACE_ROOT
  INCLUDES += -I$(ACE_ROOT) -I$(TAO_ROOT) -I$(TAO_ROOT)/orbsvcs
  LDFLAGS  += -L$(ACE_ROOT)/lib
endif

# Inclusões para distribuições Linux (Debian / Ubuntu / CentOS)
INCLUDES += -I/usr/include/tao -I/usr/include/ace -I/usr/include/tao/orbsvcs

# Bibliotecas necessárias do TAO e ACE
LIBS = -lTAO_CosNaming -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE -lpthread

# Executável de destino
TARGET = $(BIN_DIR)/cliente

# Arquivo IDL e stubs gerados para o cliente
IDL = $(IDL_DIR)/Logger.idl
IDL_SRCS = LoggerC.cpp
IDL_HDRS = LoggerC.h LoggerC.inl

# Arquivos fonte e objetos
SRCS = client.cpp $(IDL_SRCS)
OBJS = $(SRCS:.cpp=.o)

# Regra principal
all: $(TARGET)

$(TARGET): $(IDL_SRCS) $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@ln -sf cliente $(BIN_DIR)/client 2>/dev/null || cp $(TARGET) $(BIN_DIR)/client 2>/dev/null || true
	@echo "======================================================"
	@echo " Cliente compilado com sucesso: $(TARGET)"
	@echo "======================================================"

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Geração dos stubs pelo compilador TAO IDL
$(IDL_SRCS) $(IDL_HDRS): $(IDL)
	tao_idl -I$(IDL_DIR) $(IDL)

# Compilação dos objetos C++
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o $(IDL_SRCS) $(IDL_HDRS) LoggerS.* LoggerC_T.* LoggerS_T.*
	rm -f $(TARGET) $(BIN_DIR)/client

.PHONY: all clean
