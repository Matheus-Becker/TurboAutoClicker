# TurboClicker

**Um autoclicker de alto desempenho para Windows, com interface gráfica baseada em ImGui e suporte a teclas de atalho globais.**

---

## 📌 Visão Geral

O **TurboClicker** é uma ferramenta escrita em **C++** que permite automatizar cliques do mouse e pressionamentos de teclas com alta precisão e baixa latência. O projeto utiliza:
- **Windows API** para manipulação de inputs (mouse/teclado).
- **DirectX 11** e **ImGui** para a interface gráfica.
- **Threads de alta prioridade** para garantir precisão milissegundo a milissegundo.
- **Hooks de baixo nível** para capturar inputs do usuário.

---

## 🗂️ Estrutura do Projeto

```text
TurboClicker/
├── autoclicker.h          # Classe principal do autoclicker
├── autoclicker.cpp        # Implementação do autoclicker
├── input_hook.h          # Classe para captura de inputs (hooks)
├── input_hook.cpp        # Implementação dos hooks
├── main.cpp              # Ponto de entrada (WinMain + ImGui)
└── README.md             # Este arquivo
```

---

## 🔧 Dependências

- **Windows SDK** (para `windows.h`, `d3d11.h`, etc.)
- **ImGui** (para a interface gráfica)
- **DirectX 11** (para renderização)
- **C++17** (para `std::atomic`, `std::thread`, etc.)

---

## 📦 Como Compilar

### 1. Configuração do Ambiente
- Instale o **Visual Studio 2022** (ou superior) com suporte a **C++17**.
- Baixe os arquivos do **ImGui** e **DirectX 11** (ou use um gerenciador de pacotes como vcpkg).

### 2. Configuração do Projeto
- Crie um projeto **Windows Desktop Application** no Visual Studio.
- Adicione os arquivos `.h` e `.cpp` ao projeto.
- Configure as dependências:
  - **ImGui**: Inclua os arquivos `imgui.h`, `imgui_impl_win32.h`, `imgui_impl_dx11.h`, `imgui.cpp`, `imgui_impl_win32.cpp`, `imgui_impl_dx11.cpp`.
  - **DirectX 11**: Vincule as bibliotecas `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`.

### 3. Compilação
- Compile em modo **Release** para melhor desempenho.
- O executável será gerado na pasta de saída.

---

## 🏗️ Arquitetura

---

### 📜 `InputAction` (Estrutura)
Representa uma ação de input (tecla ou botão do mouse).

| Campo         | Tipo          | Descrição                                                                 |
|---------------|---------------|---------------------------------------------------------------------------|
| `type`        | `Type`        | Tipo da ação: `Keyboard`, `Mouse` ou `None`.                             |
| `code`        | `DWORD`       | Código virtual da tecla (`VK_*`) ou botão do mouse (`1`=esquerdo, `2`=direito, `3`=meio). |
| `isExtended`  | `bool`        | Indica se a tecla é estendida (ex: `VK_RMENU`).                         |

---

### 🔄 `InputHook` (Classe)
Gerencia **hooks de baixo nível** para capturar inputs do teclado e mouse.

#### **Membros Estáticos**
| Membro               | Tipo                          | Descrição                                                                 |
|----------------------|-------------------------------|---------------------------------------------------------------------------|
| `kbHook`             | `HHOOK`                       | Handle do hook do teclado.                                               |
| `msHook`             | `HHOOK`                       | Handle do hook do mouse.                                                 |
| `captureMode`        | `std::atomic<bool>`           | Modo de captura ativo (true = próximo input será capturado).             |
| `onCapture`          | `std::function<void(InputAction)>` | Callback chamado quando um input é capturado.                     |

#### **Métodos**
| Método               | Descrição                                                                 |
|----------------------|---------------------------------------------------------------------------|
| `Install()`          | Instala os hooks de teclado e mouse.                                     |
| `Uninstall()`        | Remove os hooks.                                                         |
| `LowLevelKeyboardProc` | Callback do hook do teclado (captura teclas pressionadas).              |
| `LowLevelMouseProc`   | Callback do hook do mouse (captura cliques).                              |

---

### ⚡ `Autoclicker` (Classe)
Classe principal que gerencia o **envio de inputs automatizados**.

#### **Membros Públicos**
| Membro               | Tipo                          | Descrição                                                                 |
|----------------------|-------------------------------|---------------------------------------------------------------------------|
| `running`            | `std::atomic<bool>`           | Indica se o autoclicker está ativo.                                      |
| `repeatRateMs`       | `std::atomic<int>`            | Intervalo entre repetições (em milissegundos).                          |
| `burstCount`         | `std::atomic<int>`            | Número de ações por repetição.                                           |
| `action`             | `InputAction`                | Ação a ser repetida (tecla ou botão do mouse).                           |
| `holdMode`           | `std::atomic<bool>`           | Modo "hold" (ativo enquanto a tecla de toggle estiver pressionada).     |

#### **Métodos**
| Método               | Descrição                                                                 |
|----------------------|---------------------------------------------------------------------------|
| `Start()`            | Inicia o autoclicker (cria uma thread de trabalho).                      |
| `Stop()`             | Para o autoclicker (mas não encerra a thread).                           |
| `Shutdown()`         | Encerra a thread de trabalho.                                             |
| `SendInputAction()`  | Envia uma ação de input (tecla ou mouse) para o sistema.                 |
| `SendBurst()`        | Envia uma sequência de ações (ex: 5 cliques seguidos).                   |
| `Worker()`           | Thread de trabalho que envia os inputs em loop.                          |

---

### 🖥️ `main.cpp`
Ponto de entrada da aplicação. Inicializa:
- **Janela Windows** (com `WinMain`).
- **DirectX 11** e **ImGui** para a interface gráfica.
- **Hooks de input** para captura de teclas/botões.
- **Hotkey global** (padrão: `F9`) para ligar/desligar o autoclicker.

#### **Interface Gráfica (ImGui)**
- **Sliders** para ajustar `repeatRateMs` e `burstCount`.
- **Botão de captura** para definir a ação a ser repetida.
- **Seletor de modo**: Toggle (liga/desliga com uma tecla) ou Hold (ativo enquanto a tecla estiver pressionada).
- **Botão INICIAR/PARAR** para controle manual.

---

## 🎯 Funcionalidades

| Funcionalidade               | Descrição                                                                 |
|------------------------------|---------------------------------------------------------------------------|
| **Autoclicker**              | Repete cliques do mouse ou teclas com alta precisão.                     |
| **Modo Toggle**              | Liga/desliga o autoclicker com uma tecla (padrão: `F9`).                 |
| **Modo Hold**                | Autoclicker ativo apenas enquanto a tecla de toggle estiver pressionada.|
| **Captura de Input**         | Permite selecionar qual tecla ou botão do mouse será repetido.           |
| **Ajuste de Velocidade**     | Controle do intervalo entre repetições (1ms a 1000ms).                  |
| **Ajuste de Burst**          | Número de ações por repetição (1 a 100).                                 |
| **Hotkey Personalizável**    | Permite alterar a tecla de toggle (ex: `VK_F10`).                        |

---

## 🔐 Detalhes Técnicos

### 🎮 **Manipulação de Inputs**
- **Mouse**: Usa `mouse_event` (depreciado, mas rápido) para enviar cliques.
- **Teclado**: Usa `SendInput` para enviar teclas (suporte a teclas estendidas).
- **Precisão**: A thread de trabalho usa **busy-wait** (spin lock) para evitar atrasos causados por `Sleep`.

### 🪣 **Hooks de Baixo Nível**
- **`WH_KEYBOARD_LL`**: Captura teclas pressionadas globalmente.
- **`WH_MOUSE_LL`**: Captura cliques do mouse globalmente.
- **Bloqueio de Input**: Quando `captureMode` está ativo, o próximo input é capturado e **bloqueado** (não é passado para o sistema).

### 🧵 **Thread de Trabalho**
- Prioridade: **`THREAD_PRIORITY_TIME_CRITICAL`** (máxima prioridade para precisão).
- Loop principal:
  1. Verifica se `running` está ativo.
  2. Envia um **burst** de ações (ex: 5 cliques).
  3. Aguarda o intervalo `repeatRateMs` (usando busy-wait).

---

## ⚠️ Limitações e Avisos

1. **`mouse_event` está depreciado**:
   - A Microsoft recomenda usar `SendInput` para mouse, mas `mouse_event` é mais rápido e simples para este caso.
   - Em versões futuras do Windows, pode ser removido.

2. **Hooks Globais**:
   - Requer permissões de administrador para funcionar corretamente em alguns sistemas.
   - Pode conflitar com outros programas que usam hooks (ex: antivírus).

3. **Uso de CPU**:
   - O **busy-wait** consome CPU enquanto aguarda o intervalo. Para uso prolongado, considere otimizações.

4. **Segurança**:
   - Este tipo de ferramenta pode ser detectado como **malware** por alguns antivírus (falsos positivos).
   - **Não use em jogos online** (pode ser considerado trapaça e resultar em banimento).

---

## 📜 Licença

Este projeto é **open-source** e pode ser usado livremente para fins educacionais ou pessoais.
**Não nos responsabilizamos pelo uso indevido.**

---

## 🤝 Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para:
- Reportar bugs.
- Sugerir melhorias.
- Enviar pull requests.

---
