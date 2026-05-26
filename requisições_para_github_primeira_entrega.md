O que o repositório GitHub precisa ter
Estrutura obrigatória
├── README.md
├── /docs
├── /applications
├── /esp32-esp8266
└── /schematics

Conforme especificado no documento:

Detalhamento de cada pasta
README.md

O README deve conter:

Nome do projeto
Tema escolhido (MNR)
Objetivo do sistema
Integrantes do grupo
Tecnologias utilizadas
Arquitetura do projeto
Fluxo de comunicação
Como executar:
firmware ESP32
backend
frontend/dashboard
broker MQTT
Prints/imagens do sistema
Organização das pastas
Status atual do projeto
Próximos passos

O documento destaca que o GitHub será avaliado pela:

organização;
documentação;
commits colaborativos;
README detalhado.
/docs

Deve conter:

Relatório parcial no modelo MNR/ABNT2
Imagens do projeto
Diagramas
Arquitetura do sistema
Fluxo MQTT
Prints do dashboard

Mesmo sendo checkpoint, já é importante iniciar a documentação.

/applications

Colocar:

Backend

Exemplos:

Flask
Django
Node.js
API REST
MQTT subscriber/publisher
Frontend / Dashboard

Exemplos:

React
HTML/CSS/JS
Node-RED
Dashboard web

No checkpoint, o documento pede:

protótipo do frontend;
dashboard inicial.
/esp32-esp8266

Deve conter:

Código do firmware ESP32
Configuração Wi-Fi
Comunicação MQTT
Leitura dos sensores
Controle de atuadores
Organização por módulos/arquivos

O projeto obrigatoriamente deve usar:

ESP32;
sensores/atuadores;
Wi-Fi;
MQTT.
/schematics

Adicionar:

Diagramas eletrônicos
Protótipos do circuito
Fritzing
Wokwi
KiCad
Tinkercad
Esquemas de ligação dos sensores

O checkpoint avalia:

organização do circuito;
uso correto dos sensores/atuadores.
O que precisa estar funcionando até 26/05
Obrigatório demonstrar
Protótipo funcional
ESP32 ligado
Sensor funcionando
Comunicação MQTT funcionando
Fluxo de comunicação

Exemplo:

ESP32 → MQTT Broker → Backend → Dashboard
Dashboard inicial

Mesmo simples, precisa:

mostrar dados em tempo real;
receber dados do ESP32.
Arquitetura definida

Mostrar:

como os módulos se comunicam;
infraestrutura local/nuvem;
onde fica o broker MQTT.
Código organizado

Separação entre:

firmware;
backend;
frontend.
Commits no GitHub

Ideal:

commits frequentes;
commits de vários integrantes.
Itens que serão avaliados no checkpoint
1. Prototipagem
Sensores e atuadores funcionando
Circuito organizado
2. Artefatos de desenvolvimento
Código frontend
Código backend
Firmware ESP32
3. Arquitetura do projeto
Fluxo de comunicação
Infraestrutura local/nuvem
4. Documentação
Relatório parcial
GitHub organizado
5. Aplicação
Dashboard
Protótipo frontend

Tecnologias esperadas no projeto
Hardware
ESP32
Sensores
LEDs/atuadores
Comunicação
Wi-Fi
MQTT
Software
PlatformIO
FreeRTOS
Broker MQTT (Mosquitto)
Backend
Dashboard Web

O que aumenta a qualidade da entrega
Muito recomendado
README com:
GIFs
prints
diagramas
vídeo curto
Organização de código

Exemplo:

/applications
    /backend
    /frontend

/esp32-esp8266
    /src
    /include
Arquivo .env.example

Para facilitar configuração.

Documentação da API/MQTT

Exemplo:

Tópico MQTT:
casa/sala/temperatura
Fluxograma/arquitetura

Imagem mostrando:

ESP32 → Broker MQTT → Backend → Frontend
Resumo do mínimo necessário para o checkpoint
O repositório precisa ter:
README detalhado
Código do ESP32
Backend inicial
Dashboard inicial
Diagramas do circuito
Estrutura organizada
Commits do grupo
Comunicação MQTT funcionando
Protótipo demonstrável
Observação importante

O checkpoint vale apenas 5 pontos, mas ele influencia diretamente:

a organização do projeto;
a evolução da entrega final;
a avaliação contínua dos professores