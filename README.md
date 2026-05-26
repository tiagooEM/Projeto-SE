# PowerGuard IoT – Sistema Inteligente de Detecção de Cortes e Anomalias Energéticas

Este repositório contém a infraestrutura e o dashboard do projeto **PowerGuard IoT**, desenvolvido para a disciplina de Sistemas Embarcados do CESAR School. O sistema monitora a corrente elétrica em tempo real para detectar cortes de energia e anomalias energéticas.

## 🛠️ Ferramentas Utilizadas

O projeto está totalmente conteinerizado utilizando **Docker**, dividindo-se nos seguintes microsserviços:

* **React (Vite):** Dashboard frontend moderno para visualização dos dados em tempo real.
* **Eclipse Mosquitto:** Broker MQTT responsável pela mensageria do sistema (com suporte a WebSockets).
* **Python (Simulador):** Script que simula o comportamento do sensor SCT-013 e do ESP32 enviando payloads JSON.
* **Node-RED:** Ferramenta de backend para automação e fluxos de dados paralelos.

## 🚀 Como Rodar o Projeto

Certifique-se de ter o **Docker** instalado na sua máquina.

1. Abra o terminal na pasta raiz do projeto (onde está o arquivo `docker-compose.yml`).
2. Execute o comando abaixo para construir as imagens e iniciar todos os serviços em segundo plano:
   ```bash
   docker compose up -d --build
