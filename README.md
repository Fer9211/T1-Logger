# Trabalho 1: Sistema Cliente/Servidor CORBA - Logger

**Disciplina:** Programação Distribuída / Computação Distribuída  
**Integrantes do Grupo:**

- Andressa de Oliveira Barros
- Lissa Deguti
- Melissa Weiss Perussulo
- Fernanda Costa Moraes

---

## 📌 1. Visão Geral do Projeto

Este projeto consiste na implementação de um serviço distribuído de **Logger** utilizando a arquitetura **CORBA (Common Object Request Broker Architecture)** e o middleware **TAO (The ACE ORB)** em C++.

O objetivo do sistema é permitir que múltiplos clientes distribuídos pela rede notifiquem eventos e erros de forma assíncrona para um servidor centralizado (**Logger**), o qual registra os eventos, imprime-os em tela em tempo real e permite consultas do último endereço de ocorrência de cada severidade.

```
       [ Cliente 1 ] ---> (log assíncrono: oneway) ---> +-------------------+
                                                        |   Servidor Logger |
       [ Cliente 2 ] ---> (log assíncrono: oneway) ---> |   - Imprime tela  |
                                                        |   - Guarda no map |
       [ Cliente N ] <--- (locate: último endereço) <-- +-------------------+
                                                                  ^
                                                                  | IOR
                                                       +--------------------+
                                                       |    NameService     |
                                                       |  ("LoggerService") |
                                                       +--------------------+
```

---

## 📋 2. Requisitos Atendidos do Enunciado

| Requisito do Enunciado           | Implementação no Projeto                                                            | Arquivo de Referência                    |
| :------------------------------- | :---------------------------------------------------------------------------------- | :--------------------------------------- |
| **Operação `log()` assíncrona**  | Definida com o modificador `oneway void` na IDL                                     | `idl/Logger.idl`                         |
| **Enum de Severidade**           | `enum Severidade { DEBUG, WARNING, ERROR, CRITICAL };`                              | `idl/Logger.idl`                         |
| **Parâmetros exatos de `log()`** | `severidade`, `endereco` (string), `pid` (ushort), `hora` (long), `msg` (string)    | `idl/Logger.idl`                         |
| **Impressão na tela**            | O método `log()` imprime todos os dados recebidos a cada invocação remota           | `servidor/LoggerI.cpp`                   |
| **Método `locate(s)`**           | Retorna o último endereço registrado para a severidade informada                    | `servidor/LoggerI.cpp`                   |
| **Tratamento de Exceção**        | Lança `EventNotFound` caso a severidade consultada ainda não tenha ocorrências      | `idl/Logger.idl`, `servidor/LoggerI.cpp` |
| **Servidor de Nomes**            | O servidor publica sua referência IOR sob o nome `"LoggerService"` no `CosNaming`   | `servidor/servidor.cpp`                  |
| **Dados Fictícios no Cliente**   | O cliente envia logs simulando múltiplos processos e IPs na rede                    | `cliente/client.cpp`                     |
| **Bateria Completa de Testes**   | O cliente testa 100% da interface definida (exceção, envio, consulta e atualização) | `cliente/client.cpp`                     |
| **Identificação do Grupo**       | Nomes dos 4 integrantes presentes em comentário no início da IDL                    | `idl/Logger.idl`                         |

---

## 📂 3. Estrutura de Diretórios e Papel de Cada Arquivo

```
T1-Logger/
├── Makefile               # Makefile raiz para orquestrar a compilação geral
├── README.md              # Documentação completa e guia de estudo
├── .gitignore             # Arquivos intermediários ignorados pelo controle de versão
├── bin/                   # Diretório onde os binários finais são colocados
│   ├── servidor           # Executável do servidor
│   └── cliente            # Executável do cliente de testes
├── idl/
│   └── Logger.idl         # Contrato de interface CORBA (IDL)
├── servidor/
│   ├── LoggerI.h          # Declaração da classe serva (Servant) LoggerI
│   ├── LoggerI.cpp        # Implementação dos métodos log() e locate()
│   ├── servidor.cpp       # Código do servidor: inicialização do ORB, POA e NameService
│   ├── Logger.h           # Header auxiliar de compatibilidade
│   └── Makefile           # Automação de compilação do servidor e geração dos skeletons
└── cliente/
    ├── client.cpp         # Código do cliente que executa todos os testes exigidos
    └── Makefile           # Automação de compilação do cliente e geração dos stubs
```

---

## 🔍 4. Entendendo o Código Passo a Passo

### 4.1. O Contrato IDL (`idl/Logger.idl`)

O arquivo IDL define os tipos e as operações que o cliente pode invocar remotamente:

- **`module T1Logger`**: cria um namespace em C++ para evitar conflitos de nomes.
- **`enum Severidade`**: restringe os valores aceitos a `DEBUG`, `WARNING`, `ERROR` e `CRITICAL`.
- **`exception EventNotFound`**: exceção lançada caso o método `locate()` não encontre eventos para a severidade consultada.
- **`oneway void log(...)`**: a palavra-chave `oneway` define que a chamada é assíncrona. O cliente faz a requisição e continua sua execução imediatamente, sem bloquear.
- **`string locate(...) raises (EventNotFound)`**: método síncrono que retorna uma string (`char*` em C++) ou dispara a exceção declarada.

### 4.2. O Servant (`servidor/LoggerI.h` e `servidor/LoggerI.cpp`)

O Servant é o objeto C++ real que processa as chamadas remotas:

- Herda de `POA_T1Logger::Logger` (esqueleto gerado pelo compilador IDL).
- Utiliza um `std::map<T1Logger::Severidade, std::string> ultimosEnderecos` para mapear cada severidade ao endereço do seu evento mais recente.
- **Thread-Safety**: utiliza `std::lock_guard<std::mutex>` tanto em `log()` quanto em `locate()`, garantindo que múltiplos clientes possam chamar o servidor concorrentemente sem causar corrupção de memória.
- **Gerenciamento de Memória CORBA**: no método `locate()`, o retorno utiliza `CORBA::string_dup(...)`. Pelas regras de CORBA C++, o servant deve duplicar a string na memória dinâmica para que o skeleton do ORB possa desalocá-la com segurança após o envio da resposta.

### 4.3. O Programa Servidor (`servidor/servidor.cpp`)

Executa o fluxo canônico de inicialização CORBA:

1. `CORBA::ORB_init`: inicializa a infraestrutura do ORB.
2. `resolve_initial_references("RootPOA")` e ativação do `POAManager`: prepara o gerenciador de adaptadores de objetos.
3. Instancia o servant `LoggerI`.
4. Ativa o servant no RootPOA através de `activate_object`, obtendo sua referência IOR.
5. `resolve_initial_references("NameService")`: conecta-se ao Servidor de Nomes e registra o serviço com o nome `"LoggerService"` via `rebind()`.
6. `orb->run()`: entra em loop para aguardar e processar as chamadas remotas dos clientes.

### 4.4. O Cliente de Testes (`cliente/client.cpp`)

Desenvolvido de forma limpa, direta e sequencial:

1. Inicializa o ORB e resolve `"LoggerService"` no Servidor de Nomes.
2. Converte a referência genérica com `T1Logger::Logger::_narrow(...)`.
3. Executa a suíte de testes:
   - **Teste 1 (Exceção)**: invoca `locate(CRITICAL)` antes de qualquer log para comprovar o disparo e captura da exceção `EventNotFound`.
   - **Teste 2 (Envio assíncrono)**: dispara logs fictícios para todas as severidades (`DEBUG`, `WARNING`, `ERROR`, `CRITICAL`) com endereços no formato `"ip:porta"`, PIDs e mensagens descritivas.
   - **Teste 3 (Validação de locate)**: consulta `locate()` para cada severidade e verifica se os endereços conferem com os que foram enviados.
   - **Teste 4 (Atualização dinâmica)**: envia um novo evento de `WARNING` vindo de outro endereço (`10.0.0.99:9999`) e comprova que o `locate(WARNING)` atualiza para o novo endereço.
4. Libera a memória com `CORBA::String_var` (que desaloca automaticamente strings de retorno CORBA) e encerra o ORB.

---

## 🎓 5. Guia para Apresentação e Defesa do Trabalho

Caso algum integrante do grupo seja sorteado pelo professor para a apresentação, aqui estão as principais perguntas conceituais e como respondê-las com clareza:

### 1. Por que a operação `log()` é declarada como `oneway`?

> _"O enunciado especifica que o Logger recebe notificações assíncronas. Em CORBA IDL, o modificador `oneway` indica que a chamada não é bloqueante: o cliente envia a mensagem e continua imediatamente sem esperar resposta do servidor, e a operação obrigatoriamente não pode ter parâmetros de saída nem valor de retorno."_

### 2. Por que usamos `CORBA::string_dup` no servidor e `CORBA::String_var` no cliente?

> _"No mapeamento CORBA para C++, a gestão de memória de tipos gerenciados (como strings) segue regras rígidas de transferência de posse:_
> _- **No servidor:** o método `locate()` retorna um ponteiro `char_`. Devemos usar `CORBA::string_dup(str)`para alocar a string na heap de acordo com o gerenciador do CORBA. O esqueleto (skeleton) do ORB assume a posse dessa memória e a desaloca após transmiti-la.*
*- **No cliente:** recebemos a string dentro de um`CORBA::String_var`. Esse tipo funciona como um smart pointer (`std::unique_ptr`), desalocando automaticamente a memória recebida ao sair do escopo."\*

### 3. Para que serve o método `_narrow(...)`?

> _"Em CORBA, todas as referências remotas obtidas inicialmente (como pelo Naming Service ou pelo RootPOA) vêm com o tipo genérico `CORBA::Object_ptr`. O método `_narrow` realiza um 'downcast seguro em tempo de execução', consultando o repositório de interfaces ou os metadados do objeto para garantir que aquela referência realmente implementa a interface esperada (`Logger` ou `NamingContext`)."_

### 4. Por que foi utilizado um `std::mutex` no Servant `LoggerI`?

> _"Porque servidores CORBA podem executar em ambientes multithread, onde o ORB despacha chamadas de múltiplos clientes simultaneamente em threads distintas. O mutex garante a exclusão mútua ao ler ou escrever no `std::map ultimosEnderecos`, evitando condições de corrida."_

### 5. Qual é o papel do Servidor de Nomes (`CosNaming`)?

> _"O Servidor de Nomes funciona como uma lista telefônica distribuída (DNS de objetos). Em vez do cliente precisar conhecer a IOR (Interoperable Object Reference) codificada em texto ou o endereço físico direto do servidor, o servidor registra sua IOR associada a um nome amigável (`'LoggerService'`). O cliente consulta esse nome para obter a referência do objeto remoto com total transparência de localização."_

---

## ⚙️ 6. Pré-requisitos

Para compilar e executar o projeto em ambiente Linux (ou máquina virtual/servidor da universidade):

- Compilador C++: `g++` (suporte a C++11 ou superior)
- `make`
- `TAO` (The ACE ORB) e ferramentas IDL:
  - No Ubuntu/Debian:
    ```bash
    sudo apt-get update
    sudo apt-get install g++ make libace-dev libtao-dev tao-idl tao-naming
    ```

---

## 🔨 7. Como Compilar

### Opção Recomendada: Compilar tudo pela raiz

Na pasta principal do projeto (`T1-Logger`), execute:

```bash
make
```

### Compilar individualmente:

Para o servidor:

```bash
cd servidor
make
```

Para o cliente:

```bash
cd cliente
make
```

### Limpar os arquivos gerados e binários:

```bash
make clean
```

Os binários finais são gerados automaticamente na pasta `bin/`:

- `bin/servidor`
- `bin/cliente`

---

## 🚀 8. Como Executar

Recomenda-se abrir **3 terminais distintos**:

### Terminal 1: Iniciar o Serviço de Nomes (Naming Service do TAO)

```bash
tao_cosnaming -m 1
```

_(Caso utilize a variável de ambiente `$TAO_ROOT`: `$TAO_ROOT/orbsvcs/Naming_Service/tao_cosnaming -m 1`)_

> **Nota de Porta:** Caso o Naming Service esteja rodando em uma porta específica (ex: 12345), basta adicionar `-ORBInitRef NameService=corbaloc:iiop:localhost:12345/NameService` ao executar tanto o servidor quanto o cliente.

### Terminal 2: Iniciar o Servidor Logger

Na raiz do projeto:

```bash
./bin/servidor
```

_Saída esperada:_

```text
Servidor Logger iniciado com sucesso e registrado no NameService.
Aguardando chamadas dos clientes...
```

### Terminal 3: Executar o Cliente de Testes

Na raiz do projeto:

```bash
./bin/cliente
```

_O cliente executará os 4 testes sequenciais, exibindo o resultado das asserções no terminal 3 e os logs formatados sendo impressos em tempo real no terminal 2 do servidor._

---

## 📤 9. Arquivos para Submissão

Conforme especificado no enunciado, os arquivos necessários para envio são:

1. `idl/Logger.idl`
2. `servidor/LoggerI.cpp`
