import React, { useEffect, useState } from 'react';
import mqtt from 'mqtt';

function App() {
  const [corrente, setCorrente] = useState(0);
  const [status, setStatus] = useState("Aguardando...");
  const [conectado, setConectado] = useState(false);

  useEffect(() => {
    // Conecta ao broker local na porta mapeada (9002) via WebSocket
    const client = mqtt.connect('ws://localhost:9002');

    client.on('connect', () => {
      console.log('Conectado ao Broker!');
      setConectado(true);
      client.subscribe('powerguard/sensores');
    });

    client.on('message', (topic, message) => {
      const payload = JSON.parse(message.toString());
      setCorrente(payload.corrente);
      setStatus(payload.status);
    });

    client.on('error', (err) => {
      console.error('Erro de conexão:', err);
      client.end();
    });

    return () => {
      if (client) client.end();
    };
  }, []);

  return (
    <div style={{ fontFamily: 'Arial, sans-serif', textAlign: 'center', padding: '50px' }}>
      <h1>⚡ PowerGuard IoT Dashboard</h1>
      
      <div style={{
        margin: '0 auto',
        padding: '30px', 
        border: status === 'corte' ? '2px solid #c62828' : '1px solid #ccc', 
        borderRadius: '10px', 
        width: '350px',
        backgroundColor: status === 'corte' ? '#ffebee' : '#e8f5e9',
        boxShadow: '0 4px 8px rgba(0,0,0,0.1)',
        transition: 'all 0.3s'
      }}>
        <h3 style={{ margin: '0 0 20px 0', color: '#555' }}>Corrente RMS</h3>
        <h2 style={{ fontSize: '48px', margin: '0', color: '#2c3e50' }}>{corrente.toFixed(2)} A</h2>
        
        <h3 style={{ 
          marginTop: '20px', 
          color: status === 'corte' ? '#c62828' : '#2e7d32' 
        }}>
          {status === 'corte' ? '⚠ CORTE DETECTADO' : '✅ ENERGIA NORMAL'}
        </h3>
      </div>

      <p style={{ marginTop: '30px', color: conectado ? 'green' : 'red' }}>
        {conectado ? '🟢 Conectado ao Broker' : '🔴 Desconectado'}
      </p>
    </div>
  );
}

export default App;