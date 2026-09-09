"use client";

import { useEffect, useState } from "react";

interface Sensor {
  id: number;
  name: string;
  location: string;
}

interface SensorReading {
  "sensor-id": number;
  value: number;
  timestamp: string;
}

export default function Home() {
  const [sensors, setSensors] = useState<Sensor[]>([]);
  const [readings, setReadings] = useState<Record<number, SensorReading[]>>(
    {}
  );
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchSensors = async () => {
      try {
        const res = await fetch("/sensors");
        if (!res.ok) throw new Error("Failed to fetch sensors");
        const data = await res.json();
        setSensors(data.sensors);

        const readingsMap: Record<number, SensorReading[]> = {};
        await Promise.all(
          data.sensors.map(async (sensor: Sensor) => {
            const rRes = await fetch(`/sensors/${sensor.id}/readings`);
            if (rRes.ok) {
              const rData = await rRes.json();
              readingsMap[sensor.id] = rData.readings || [];
            } else {
              readingsMap[sensor.id] = [];
            }
          })
        );
        setReadings(readingsMap);
      } catch (err) {
        setError(err instanceof Error ? err.message : "Unknown error");
      } finally {
        setLoading(false);
      }
    };

    fetchSensors();
  }, []);

  if (loading) {
    return (
      <div className="flex min-h-screen items-center justify-center">
        <p className="text-lg">Loading sensors...</p>
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex min-h-screen items-center justify-center">
        <p className="text-lg text-red-500">Error: {error}</p>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-50 p-8">
      <h1 className="text-3xl font-bold mb-8 text-center text-gray-800">Sensor Readings</h1>
      <div className="max-w-4xl mx-auto grid gap-6">
        {sensors.map((sensor) => (
          <div
            key={sensor.id}
            className="bg-white rounded-lg shadow-md p-6"
          >
            <div className="flex justify-between items-center mb-4">
              <h2 className="text-xl font-semibold text-gray-800">{sensor.name}</h2>
              <span className="text-sm text-gray-800">
                {sensor.location}
              </span>
            </div>
            {readings[sensor.id]?.length ? (
              <div className="space-y-2">
                {readings[sensor.id].map((reading, idx) => (
                  <div
                    key={idx}
                    className="flex justify-between items-center border-b pb-2"
                  >
                    <span className="font-mono text-lg text-gray-800">
                      {reading.value}
                    </span>
                    <span className="text-sm text-gray-800">
                      {new Date(reading.timestamp).toLocaleString()}
                    </span>
                  </div>
                ))}
              </div>
            ) : (
              <p className="text-gray-800 italic">No readings yet</p>
            )}
          </div>
        ))}
      </div>
    </div>
  );
}
