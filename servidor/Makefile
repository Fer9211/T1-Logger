CXX = g++
CXXFLAGS = -Wall -O2 -std=c++11

# diretorios
BIN_DIR = ../bin
IDL_DIR = ../idl

# deteccao e inclusao do ACE / TAO
INCLUDES = -I. -I$(IDL_DIR)

ifdef ACE_ROOT
  INCLUDES += -I$(ACE_ROOT) -I$(TAO_ROOT) -I$(TAO_ROOT)/orbsvcs
  LDFLAGS  += -L$(ACE_ROOT)/lib
endif

# p distribuicoes linux
INCLUDES += -I/usr/include/tao -I/usr/include/ace -I/usr/include/tao/orbsvcs

# bibliotecas do TAO e ACE
LIBS = -lTAO_CosNaming -lTAO_PortableServer -lTAO_AnyTypeCode -lTAO -lACE -lpthread

# gatilho
TARGET = $(BIN_DIR)/servidor

# IDL e stubs/skeletons
IDL = $(IDL_DIR)/Logger.idl
IDL_SRCS = LoggerC.cpp LoggerS.cpp
IDL_HDRS = LoggerC.h LoggerS.h LoggerC.inl

# fonte e objct
SRCS = servidor.cpp LoggerI.cpp $(IDL_SRCS)
OBJS = $(SRCS:.cpp=.o)

# regra principal
all: $(TARGET)

$(TARGET): $(IDL_SRCS) $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS)
	@echo "======================================================"
	@echo " Servidor compilado com sucesso: $(TARGET)"
	@echo "======================================================"

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# gera stubs/skeletons pelo compilador TAO IDL
$(IDL_SRCS) $(IDL_HDRS): $(IDL)
	tao_idl -I$(IDL_DIR) $(IDL)

# compila objetos distribuidosC++
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o $(IDL_SRCS) $(IDL_HDRS) LoggerS_T.* LoggerC_T.*
	rm -f $(TARGET)

.PHONY: all clean
