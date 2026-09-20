# Trabalho 1: Sistema Cliente/Servidor CORBA - Logger

**Disciplina:** Programação Distribuída / Computação Distribuída  
**Integrantes do Grupo:**
- Fernanda Costa Moraes
- Andressa de Oliveira Barros
- Lissa Deguti
- Melissa Weiss Perussulo

---

## 📌 Descrição do Projeto

Sistema cliente/servidor baseado na arquitetura **CORBA (Common Object Request Broker Architecture)** utilizando **TAO (The ACE ORB)**. O servidor atua como um serviço de **Logger**, registrando notificações de eventos assíncronos enviados por múltiplos clientes espalhados na rede.

### Funcionalidades implementadas:
1. **Interface IDL (`Logger.idl`)**:
   - `enum Severidade`: `DEBUG`, `WARNING`, `ERROR`, `CRITICAL`.
   - `exception EventNotFound`: disparada quando uma severidade buscada ainda não possui registros.
   - `attribute boolean verbose`: ativa ou desativa a exibição em tempo real das mensagens recebidas no terminal do servidor.
   - `oneway void log(...)`: operação assíncrona para envio de logs (severidade, endereço `ip:porta`, PID do processo, hora/timestamp em segundos e mensagem descritiva).
   - `string locate(in Severidade s)`: retorna o endereço `ip:porta` do último evento recebido com a severidade especificada, ou lança `EventNotFound`.

2. **Servidor (`servidor/`)**:
   - Inicializa o ORB e o RootPOA com POAManager.
   - Implementa o servant na classe `LoggerI` (`LoggerI.h` e `LoggerI.cpp`) com sincronização por mutex (`std::mutex`) para garantir segurança com chamadas concorrentes.
   - Publica o serviço no Servidor de Nomes (**CosNaming**) sob o nome `"LoggerService"`.

3. **Cliente de Testes (`cliente/client.cpp`)**:
   - Conecta-se ao **NameService** e obtém a referência do `LoggerService`.
   - **Teste 1 (Exceção)**: Consulta `locate()` para severidades ainda não registradas e valida o disparo de `EventNotFound`.
   - **Teste 2 (Atributo verbose)**: Lê e altera o atributo `verbose`.
   - **Teste 3 (Múltiplos Clientes)**: Simula o envio de dados fictícios por diversos clientes espalhados na rede com severidades variadas, IPs no formato `"ip:porta"` (ex: `"192.168.1.1:1500"`), PIDs, timestamps e mensagens.
   - **Teste 4 (Verificação de locate)**: Consulta e valida o último endereço registrado para cada severidade.
   - **Teste 5 (Alternância de verbose)**: Desliga o verbose (modo silencioso), envia logs, e reativa o verbose.

---

## 📂 Estrutura de Diretórios

```
T1-Logger/
├── Makefile               # Makefile raiz para compilar servidor e cliente
├── README.md              # Este documento
├── .gitignore             # Arquivos ignorados pelo Git
├── bin/                   # Diretório de destino dos executáveis gerados
│   ├── servidor           # Executável do servidor
│   └── cliente            # Executável do cliente
├── idl/
│   └── Logger.idl         # Contrato IDL do serviço Logger
├── servidor/
│   ├── LoggerI.h          # Declaração da classe serva LoggerI
│   ├── LoggerI.cpp        # Implementação dos métodos do servant
│   ├── servidor.cpp       # Código principal do servidor (ORB + NameService)
│   ├── Logger.h           # Header auxiliar de compatibilidade
│   └── Makefile           # Automação de compilação do servidor
└── cliente/
    ├── client.cpp         # Código do cliente de testes
    └── Makefile           # Automação de compilação do cliente
```

---

## ⚙️ Pré-requisitos

Para compilar e executar o projeto em ambiente Linux (ou máquina virtual/laboratório da universidade), certifique-se de ter instalado o **ACE+TAO** e ferramentas de compilação C++:

- `g++` (com suporte a C++11 ou superior)
- `make`
- `tao_idl` (compilador IDL do TAO)
- `TAO` e `CosNaming` (pacotes `libace-dev`, `libtao-dev`, `tao-idl`, `tao-naming` no Debian/Ubuntu)

---

## 🔨 Como Compilar

### Opção 1: Compilar tudo pela raiz (Recomendado)
Na pasta raiz do projeto (`T1-Logger`), execute:
```bash
make
```

### Opção 2: Compilar individualmente em cada pasta
Para compilar apenas o servidor:
```bash
cd servidor
make
```

Para compilar apenas o cliente:
```bash
cd cliente
make
```

Para limpar os arquivos compilados e gerados:
```bash
make clean
```

Os executáveis serão gerados automaticamente na pasta `bin/` (`bin/servidor` e `bin/cliente`).

---

## 🚀 Como Executar

Abra 3 terminais separados (ou use abas/tmux):

### Terminal 1: Iniciar o Serviço de Nomes (Naming Service do TAO)
```bash
tao_cosnaming -m 1
# Ou:
$TAO_ROOT/orbsvcs/Naming_Service/tao_cosnaming -m 1
```

> **Nota:** Se o NameService estiver rodando em uma porta específica (ex: 12345), adicione `-ORBInitRef NameService=corbaloc:iiop:localhost:12345/NameService` ao iniciar o servidor e o cliente.

### Terminal 2: Iniciar o Servidor Logger
Na raiz do projeto:
```bash
./bin/servidor
```
O servidor será registrado no Naming Service como `LoggerService` e aguardará requisições.

### Terminal 3: Executar o Cliente de Testes
Na raiz do projeto:
```bash
./bin/cliente
```
O cliente executará automaticamente toda a bateria de testes, exibindo os resultados no terminal do cliente e as impressões de log no terminal do servidor.

---

## 📤 Arquivos para Envio

Conforme solicitado no enunciado da atividade, os arquivos requeridos para submissão são:
1. `idl/Logger.idl`
2. `servidor/LoggerI.cpp`
