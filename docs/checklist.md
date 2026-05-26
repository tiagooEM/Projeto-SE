# Checklist para entrega no GitHub (checkpoint)

Itens obrigatórios:
- README detalhado (preenchido em `README.md`).
- `/docs` com relatório parcial e imagens.
- `/applications/backend` e `/applications/frontend` com protótipos ou stubs.
- `/esp32-esp8266` contendo o firmware (PlatformIO).
- `/schematics` com diagramas ou fotos do circuito.

Boas práticas de commits:
- Commits frequentes e pequenos, com mensagens claras em português ou inglês.
- Cada integrante deve fazer commits (usar branches individuais e abrir PRs quando possível).
- Mensagem sugerida: `feat: adicionar leitura RMS no firmware` ou `docs: atualizar README`.

Inclusão de artefatos:
- Adicionar imagens/GIFs em `/docs/images` e referenciá-las no `README.md`.
- Incluir `.env.example` se o backend precisar de variáveis de ambiente.

Validação mínima (até 26/05):
- Protótipo funcional: placa ligada e leitura do sensor.
- Publicação MQTT testada (local ou nuvem).
- Dashboard exibindo dados em tempo real (pelo menos em protótipo).

Passos sugeridos para finalizar:
1. Preencher `README.md` com nomes dos integrantes.
2. Subir broker Mosquitto local e testar com o firmware.
3. Implementar subscriber mínimo no backend e testar fluxo até o frontend.
4. Fazer commits dos arquivos finais e abrir um PR para revisão.

Boa sorte — se quiser, posso gerar um exemplo de backend subscriber ou um dashboard protótipo agora.