const sqlite3 = require('sqlite3').verbose();
const path = require('path');

const DB_PATH = path.join(__dirname, '..', 'db', 'app.db');
const SHED_TABLE = "shedmetrics";
let db; // Use 'let' to hold the database instance later

const TYPE_TRIGGER_ONLY = 'trgevt';
const TYPE_ALLEVENTS = 'allevts';

exports.TYPE_TRIGGER_ONLY = 'trgevt';
exports.TYPE_ALLEVENTS = 'allevts';


// 1. Create a function that connects and sets up the database
const connectDB = () => {
    return new Promise((resolve, reject) => {
        db = new sqlite3.Database(DB_PATH, (err) => {
            if (err) {
                console.error('Database connection error:', err.message);
                return reject(err); // Reject the promise on connection error
            }
            console.log('Connected to the SQLite database.');

            // 2. Create the table AFTER successful connection
            db.run(`CREATE TABLE IF NOT EXISTS ${SHED_TABLE} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                inttemp TEXT NOT NULL,
                inthumidity TEXT NOT NULL,
                dewpoint TEXT NOT NULL,
                exttemp TEXT NOT NULL,
                doorstate TEXT NOT NULL,
                fanstate TEXT NOT NULL,
                blowerstate TEXT NOT NULL,
                miscstate TEXT NOT NULL,
                lightstate TEXT NOT NULL,
                trigger TEXT,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP
            )`, (createErr) => {
                if (createErr) {
                    console.error('Error creating table:', createErr.message);
                    return reject(createErr); // Reject on table creation error
                }
                console.log(`Table ${SHED_TABLE} ready.`);
                resolve(db); // Resolve the promise once everything is set up
            });
        });
    });
};

// 3. The insertion function remains the same (just updated to use the 'db' variable)
exports.newEntry = (Itemp, Ihumid, DewPoint, Etemp, door, fan, light, misc, blower, trigger_type) => {
    // Note: The logic for trimming/lowercasing should be done carefully
    const shedEntry = {
        int_temp: Itemp.trim(),
        humid: Ihumid.trim(), // Assuming you want to trim humidity too
        dew_point: DewPoint.trim(),
        out_temp: Etemp.trim(), 
        door_state: door.toLowerCase().trim(),
        fan_state: fan.toLowerCase().trim(),
        light_state: light.toLowerCase().trim(),
        misc_state: misc.toLowerCase().trim(),
        blower_state: blower.toLowerCase().trim(),
        trigger: trigger_type ? trigger_type.toLowerCase().trim() : null
    };
    const now = new Date()
    const currentSqliteTimestamp = now.toISOString();

    const sql = `INSERT INTO ${SHED_TABLE} (inttemp, inthumidity, dewpoint, exttemp, doorstate, fanstate, blowerstate, miscstate, lightstate, trigger, created_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;
    // We don't need to manually pass created_at if we use DEFAULT CURRENT_TIMESTAMP in the table definition.

    return new Promise((resolve, reject) => {
        db.run(sql, [shedEntry.int_temp, shedEntry.humid, shedEntry.dew_point, shedEntry.out_temp, shedEntry.door_state, shedEntry.fan_state, shedEntry.blower_state, shedEntry.misc_state, shedEntry.light_state, shedEntry.trigger, currentSqliteTimestamp], function (err) {
            if (err) {
                console.error('Database insertion error:', err.message);
                return reject(new Error('Database error.'));
            }
            console.log(`[Database Log] Successfully inserted entry ID: ${this.lastID}`);
            resolve(this.lastID);
        });
    });
};

exports.getData = (type) => {   
    sql = "";
    if (type === TYPE_TRIGGER_ONLY) {
        //get only trigger events
        sql = `SELECT * FROM ${SHED_TABLE} WHERE (trigger = 'interrupt' OR trigger = 'poweron') ORDER BY created_at DESC LIMIT 48`;
    }else{
        //get everything
        sql = `SELECT * FROM ${SHED_TABLE} ORDER BY created_at DESC LIMIT 48`;
    }

    console.log(`type: ${type} SQL: ${sql}`);

    
    
    return new Promise((resolve, reject) => {
            // db.all executes the SELECT query
            // The callback receives the error (err) and the array of rows (rows)
            db.all(sql, [], (err, rows) => {
                if (err) {
                    console.error('Database selection error:', err.message);
                    // Reject the promise on failure
                    return reject(new Error('Failed to retrieve data from database.'));
                }

                console.log(`[Database Log] Successfully retrieved ${rows.length} rows.`);
                // Resolve the promise, passing back the array of data objects
                resolve(rows);
            });
        });


    
}

// 4. Export the setup function so app.js can use it
exports.connect = connectDB;