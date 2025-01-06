const express = require('express');
const axios = require('axios');
const cors = require('cors');
require('dotenv').config();

const app = express();
app.use(express.json());
app.use(cors());

const thingsboardUrl = 'http://demo.thingsboard.io';
const username = process.env.THINGSBOARD_USERNAME;
const password = process.env.THINGSBOARD_PASSWORD;

// Ruta para autenticar y obtener el token
let authToken = null;
let tokenExpiration = null;

async function authenticate() {
  try {
    const response = await axios.post(`${thingsboardUrl}/api/auth/login`, {
      username,
      password,
    });
    authToken = response.data.token;
    tokenExpiration = Date.now() + 3600 * 1000;
    console.log('Autenticación exitosa');
  } catch (error) {
    console.error('Error en autenticación:', error.response?.data || error.message);
    throw new Error('No se pudo autenticar con ThingsBoard');
  }
}

// Middleware para verificar si el token es válido
app.use(async (req, res, next) => {
  if (!authToken || Date.now() > tokenExpiration) {
    await authenticate();
  }
  next();
});

// Ruta para obtener telemetría
app.get('/telemetry/:deviceId', async (req, res) => {
  const deviceId = req.params.deviceId;
  const keys = 'current_fill_level,latitude,longitude,location_name,bin_name';

  try {
    const response = await axios.get(
      `${thingsboardUrl}/api/plugins/telemetry/DEVICE/${deviceId}/values/timeseries?keys=${keys}`,
      {
        headers: { 'X-Authorization': `Bearer ${authToken}` },
      }
    );

    // Procesar los datos para Flutter
    const data = response.data;
    const result = {
      current_fill_level: parseInt(data['current_fill_level'][0]?.value || '0'),
      latitude: data['latitude'][0]?.value || '0',
      longitude: data['longitude'][0]?.value || '0',
      location_name: data['location_name'][0]?.value || 'Unknown',
      bin_name: data['bin_name'][0]?.value || 'Unknown',
    };

    res.json(result);
  } catch (error) {
    console.error('Error al obtener telemetría:', error.response?.data || error.message);
    res.status(500).json({ error: 'No se pudo obtener la telemetría' });
  }
});

// Servidor en puerto 3000
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Servidor corriendo en http://localhost:${PORT}`);
});
