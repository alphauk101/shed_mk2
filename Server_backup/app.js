// app.js
const path = require('path');
const shedEndpoint = require('./controllers/SHDEndpoint'); 
const bodyParser = require('body-parser');
const express = require('express');
const si = require('systeminformation'); // <--- CRITICAL FIX: Import systeminformation

const app = express();
const port = 3000;

// ... (Your Express middleware and routes remain the same) ...

// --- Express Configuration and Routes ---
app.set('view engine', 'ejs');
app.use(bodyParser.json());

app.get('/', (req, res) => {
  res.render('dashboard');
});

app.post('/api/shdep', async (req, res) => {
  console.log(req.body);
  const { Itemp, Ihumid, DewPoint, Etemp, DoorState, Fan, Lights, Misc, Blower, trigger } = req.body; // Extract data from the request body

  /*
  {"Itemp":"17.69","Ihumid":"70.32","DewPoint":"12.22","Etemp":"17.81","DoorState":"open","Blower":"off","Fan":"on","Misc":"off","Lights":"on"}
  */

  if (!Itemp || !Ihumid || !DewPoint || !Etemp || !DoorState || !Fan || !Lights || !Misc || !Blower || !trigger) {
    return res.status(400).json({ error: 'FIELDS MISSING' });
  }

  try {
      const newSHDentry = await shedEndpoint.newEntry (Itemp, Ihumid, DewPoint, Etemp, DoorState, Fan, Lights, Misc, Blower, trigger);

      res.status(201).json({
        message: 'OK',
        user: newSHDentry
      });

      console.log(`Entry added to DB`);
    } catch (error) {
      console.error('Error:', error);
      res.status(500).json({ error: 'FAILED' });
    }
});

app.get('/api/data', async (req, res) => {
  try{
    const envData = await shedEndpoint.getData(shedEndpoint.TYPE_ALLEVENTS);
    //console.log(envData);

      res.status(201).json({
        message: 'OK',
        data: envData
      });

  }catch (error)
  {
      console.error('Error:', error);
      res.status(500).json({ error: 'Error getting data' });
  }
});


app.get('/api/trgevt', async (req, res) => {
  try{
    const envData = await shedEndpoint.getData(shedEndpoint.TYPE_TRIGGER_ONLY);
    //console.log(envData);

      res.status(201).json({
        message: 'OK',
        data: envData
      });

  }catch (error)
  {
      console.error('Error:', error);
      res.status(500).json({ error: 'Error getting data' });
  }
});


app.get('/api/system', async (req, res) => {
    try {
          const [cpuLoad, mem, fsSize, netStats, time, cpuTemp] = await Promise.all([
              si.currentLoad(),
              si.mem(),
              si.fsSize(),
              si.networkStats(),
              si.time(),
              si.cpuTemperature()
          ]);

          // Filter for physical disks (usually starting with /dev/sd or /dev/mmc)
          // and ignore tiny system partitions
          const physicalDisks = fsSize.filter(drive => 
              drive.type !== 'tmpfs' && drive.type !== 'devtmpfs' && drive.size > 0
          ).map(drive => ({
              mount: drive.mount,
              usedPercent: drive.use,
              usedGB: (drive.used / 1024 / 1024 / 1024).toFixed(1),
              totalGB: (drive.size / 1024 / 1024 / 1024).toFixed(1)
          }));

          res.json({
              cpuLoad: cpuLoad.currentLoad.toFixed(1),
              cpuTemp: cpuTemp.main || 0,
              memUsedPct: ((mem.active / mem.total) * 100).toFixed(1),
              memTotal: (mem.total / 1024 / 1024 / 1024).toFixed(1),
              disks: physicalDisks, // Now sending an array
              netStats: netStats[0] || {},
              uptime: Math.floor(time.uptime / 3600) + "h " + Math.floor((time.uptime % 3600) / 60) + "m"
          });
      } catch (e) {
          res.status(500).json({ error: 'Failed to fetch' });
      }
  });


app.use((req, res) => {
    res.status(404).send('<h1>404 - API error</h1>');
});
// ------------------------------------

// --- Server Startup Logic ---
async function startServer() {
    try {
        console.log("Attempting to connect to database...");
        await shedEndpoint.connect(); 
        console.log("Database successfully initialized. Starting server...");


        app.listen(port, () => {
            console.log(`Express server running at http://localhost:${port}`);
        });

    } catch (error) {
        console.error("FATAL ERROR: Server failed to start due to database issues.", error);
        process.exit(1); 
    }
}

startServer(); // Call the async function to start the process