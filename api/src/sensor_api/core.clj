(ns sensor-api.core
  (:require [compojure.core :refer [defroutes GET POST]]
            [compojure.route :as route]
            [ring.middleware.json :refer [wrap-json-body wrap-json-response]]
            [ring.adapter.jetty :refer [run-jetty]]))

(def sensors
  (atom
   [{:id 1 :name "thermometer" :location "server-room"}
    {:id 2 :name "humidity-sensor" :location "greenhouse"}]))

(def sensor-readings
  (atom [{:sensor-id 1 :value 22.5 :timestamp "2026-09-09T10:00:00Z"},
    {:sensor-id 2 :value 22.5 :timestamp "2026-10-09T10:00:00Z"}]))

(defn get-all-sensors
  [_]
  {:status 200
   :body {:sensors @sensors}})

(defn get-sensor
  [{:keys [params]}]
  (let [id (some-> params :id Integer/parseInt)
        sensor (some #(when (= (:id %) id) %) @sensors)]
    (if sensor
      {:status 200 :body sensor}
      {:status 404 :body {:error "sensor not found"}})))

(defn add-sensor-reading
  [request]
  (let [body (:body request)
        sensor-id (:sensor-id body)]
    (if (and sensor-id (some #(= (:id %) sensor-id) @sensors))
      (do
        (swap! sensor-readings conj (assoc body :sensor-id sensor-id))
        {:status 201 :body body})
      {:status 400 :body {:error "invalid or unknown sensor-id"}})))

(defn get-sensor-readings
  [{:keys [params]}]
  (let [id (some-> params :id Integer/parseInt)
        sensor-readings-for-sensor (filterv #(= (:sensor-id %) id) @sensor-readings)]
    (if (some #(= (:id %) id) @sensors)
      {:status 200 :body {:readings sensor-readings-for-sensor}}
      {:status 404 :body {:error "sensor not found"}})))

(defroutes app-routes
  (GET "/sensors" [] get-all-sensors)
  (GET "/sensors/:id" [id] (fn [_] (get-sensor {:params {:id id}})))
  (GET "/sensors/:id/readings" [id] (fn [_] (get-sensor-readings {:params {:id id}})))
  (POST "/sensors/:id/readings" [id]
        (fn [request]
          (add-sensor-reading (assoc-in request [:body :sensor-id]
                                        (Integer/parseInt id)))))
  (route/not-found {:status 404 :body {:error "not found"}}))

(def app
  (-> app-routes
      (wrap-json-response)
      (wrap-json-body {:keywords? true})))

(defn -main
  [& _args]
  (println "Starting sensor API on http://localhost:3000")
  (run-jetty app {:port 3000 :join? false}))
