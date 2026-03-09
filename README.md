# PEC — STM32F767ZI (STM32CubeIDE)

Projeto embarcado para **STM32F767ZITx**, configurado no **STM32CubeIDE**, com suporte a **LWIP** e build/debug via pasta `Debug/`.

## Visão geral

Este repositório contém um firmware para a família STM32F7 com:

- Configuração de hardware via arquivo `.ioc`: [pec.ioc](pec.ioc)
- Projeto Eclipse/CubeIDE: [.project](.project), [.cproject](.cproject), [.mxproject](.mxproject)
- Linker scripts:
  - [STM32F767ZITX_FLASH.ld](STM32F767ZITX_FLASH.ld)
  - [STM32F767ZITX_RAM.ld](STM32F767ZITX_RAM.ld)
- Código da aplicação em [Core/](Core/)
- HAL/Drivers em [Drivers/](Drivers/)
- Stack de rede em [LWIP/](LWIP/) e [Middlewares/](Middlewares/)
- Artefatos de compilação em [Debug/](Debug/)

## Estrutura do projeto

- [Core/](Core/): lógica principal da aplicação, inicialização e callbacks.
- [Drivers/](Drivers/): drivers HAL/CMSIS.
- [LWIP/](LWIP/): integração da pilha TCP/IP.
- [Middlewares/](Middlewares/): componentes adicionais de middleware.
- [Debug/](Debug/): saída de build (gerada pela IDE).
- [32_64/](32_64/): arquivos de dados/recursos auxiliares do projeto.
- [.settings/](.settings/): preferências do STM32CubeIDE/Eclipse.

## Requisitos

- **STM32CubeIDE** compatível com STM32F7.
- Toolchain ARM embarcada (incluída no CubeIDE).
- ST-LINK para gravação e depuração.

## Como abrir e compilar

1. Abrir o STM32CubeIDE.
2. Importar o projeto existente usando a pasta raiz deste repositório.
3. Selecionar a configuração **Debug**.
4. Executar *Build Project*.

## Gravação e depuração

1. Conectar a placa STM32F767ZI via ST-LINK.
2. Usar a configuração de launch: [pec Debug.launch](pec%20Debug.launch).
3. Iniciar sessão de debug pela IDE.

## Observações

- Alterações de pinos, clocks e periféricos devem ser feitas em [pec.ioc](pec.ioc).
- Após regenerar código pelo CubeMX, revisar alterações em `Core/` para preservar customizações de usuário.
- Os scripts de link diferenciam execução em FLASH e RAM:
  - [STM32F767ZITX_FLASH.ld](STM32F767ZITX_FLASH.ld)
  - [STM32F767ZITX_RAM.ld](STM32F767ZITX_RAM.ld)

## Licença

Definir a licença do projeto (ex.: MIT, BSD-3-Clause, proprietário).