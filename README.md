# ⚡ PowerGuard IoT  
### Sistema Inteligente de Detecção de Cortes e Anomalias Energéticas

![Docker](https://img.shields.io/badge/Docker-Containerized-2496ED?logo=docker&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-Mosquitto-660066?logo=eclipsemosquitto&logoColor=white)
![InfluxDB](https://img.shields.io/badge/Database-InfluxDB-22ADF6?logo=influxdb&logoColor=white)
![React](https://img.shields.io/badge/Frontend-React-61DAFB?logo=react&logoColor=black)
![Python](https://img.shields.io/badge/Backend-Python-3776AB?logo=python&logoColor=white)

## 📖 Sobre o Projeto

O **PowerGuard IoT** é um sistema inteligente de monitoramento energético desenvolvido para a disciplina de **Sistemas Embarcados da CESAR School**.

O objetivo do projeto é **monitorar corrente elétrica em tempo real**, detectar **cortes de energia** e identificar **anomalias elétricas**, permitindo tanto o acompanhamento instantâneo quanto a análise histórica dos dados coletados.

Toda a infraestrutura foi construída utilizando **microsserviços conteinerizados com Docker**, garantindo portabilidade, escalabilidade e facilidade de execução.

---

## 🏗️ Arquitetura do Sistema

O sistema é composto pelos seguintes serviços:

### 🖥️ Frontend – React (Vite)
Dashboard web moderno para visualização dos dados energéticos em tempo real.

**Responsabilidades:**
- Exibição de métricas em tempo real
- Atualização via WebSockets
- Visualização de status energético
- Interface responsiva para monitoramento

---

### 📡 Broker MQTT – Eclipse Mosquitto
Responsável pela comunicação assíncrona e de baixa latência entre os componentes do sistema.

**Responsabilidades:**
- Recebimento das métricas dos sensores
- Publicação e assinatura de tópicos MQTT
- Comunicação em tempo real entre os serviços

**Tópico principal utilizado:**

```text
powerguard/sensores
```

---

### 📊 Banco de Dados – InfluxDB
Banco de dados de séries temporais (**TSDB**) otimizado para aplicações **IoT**, responsável por armazenar o histórico completo das medições elétricas.

**Responsabilidades:**
- Persistência dos dados de corrente elétrica
- Armazenamento histórico
- Base para análises futuras e geração de relatórios
- Possibilidade de integração com dashboards analíticos

---

### 🐍 Simulador – Python
Script responsável por **simular o hardware embarcado** (ESP32 + sensor SCT-013).

O simulador envia medições simultaneamente para:

- **Broker MQTT** → atualização em tempo real
- **InfluxDB** → armazenamento histórico

O sistema também simula:
- Funcionamento normal
- Oscilações de corrente
- Cortes de energia
- Eventos anômalos

## 🧱 Arquitetura Geral

```text
         ┌─────────────────────┐
         │ Simulador Python    │
         │ (ESP32 + SCT-013)   │
         └──────────┬──────────┘
                    │
        ┌───────────┴────────────┐
        │                        │
        ▼                        ▼
┌────────────────┐     ┌────────────────┐
│ Eclipse        │     │ InfluxDB       │
│ Mosquitto MQTT │     │ Time Series DB │
└────────┬───────┘     └────────────────┘
         │
         ▼
┌────────────────┐
│ React Dashboard│
│ Tempo Real     │
└────────────────┘
```

---

## 🚀 Como Executar o Projeto

### Pré-requisitos

Antes de começar, certifique-se de possuir instalado:

- Docker
- Docker Compose

Verifique:

```bash
docker --version
docker compose version
```

---

### 1️⃣ Clonar o Repositório

```bash
git clone <URL_DO_REPOSITORIO>
```

Entre na pasta do projeto:

```bash
cd powerguard-iot
```

---

### 2️⃣ Subir os Contêineres

Na raiz do projeto (onde está o arquivo `docker-compose.yml`), execute:

```bash
docker compose up -d --build
```

Esse comando irá:

- Construir as imagens Docker
- Criar a rede interna entre os serviços
- Iniciar todos os contêineres
- Configurar automaticamente as dependências

Aguarde alguns segundos para a inicialização completa.

---

## 🌐 Serviços Disponíveis

Após subir os contêineres, acesse:

| Serviço | URL |
|----------|-----|
| Dashboard React | http://localhost:5173 |
| InfluxDB | http://localhost:8086 |

---

## 🔐 Credenciais do InfluxDB

Use as seguintes credenciais para acessar o painel administrativo:

```text
Usuário: admin
Senha:   powerguard1234
```

---

## 📡 Configuração MQTT (Node-RED)

Caso queira monitorar os dados via Node-RED:

### Configuração do Broker

| Campo | Valor |
|--------|-------|
| Server | mosquitto |
| Port | 1883 |
| Protocol | MQTT V3.1.1 |
| Topic | powerguard/sensores |
| Output | parsed JSON object |

---

## 📊 Monitoramento dos Logs

Para acompanhar os eventos do simulador em tempo real:

```bash
docker logs -f powerguard-simulador
```

Você poderá visualizar:

- Envio de medições
- Eventos anômalos
- Cortes simulados de energia
- Reconexões e status do sistema

---

## 🛑 Encerrando o Projeto

Para desligar toda a infraestrutura:

```bash
docker compose down
```

> **Importante:** Os dados do banco e as configurações do Node-RED **não serão perdidos**, pois estão persistidos utilizando **volumes Docker**.

---

## 📁 Estrutura do Projeto

```text
│── applications/
│   ├── frontend/              # Dashboard React (Vite)
│   └── simulador/             # Simulador Python do ESP32 + SCT-013
│
│── docs/                      # Documentação do projeto
│
│── esp32_firmware/            # Firmware do ESP32
│
│── schematics/                # Esquemáticos e diagramas elétricos
│
│── docker-compose.yml         # Orquestração dos contêineres
│── mosquitto.conf             # Configuração do broker MQTT
│── README.md                  # Documentação principal do projeto
```

---

## 🎯 Objetivos do Projeto

- Monitorar corrente elétrica em tempo real  
- Detectar quedas/cortes energéticos  
- Persistir dados históricos  
- Permitir análises futuras de comportamento elétrico  
- Criar uma base para automações inteligentes  

---

## 👨‍💻 Equipe

Projeto desenvolvido para a disciplina de **Sistemas Embarcados – CESAR School**.

**PowerGuard IoT © 2026**
