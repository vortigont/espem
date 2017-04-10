-- Select daily stat (this one is the way how NOT to do a sql)
/*
SELECT
    DATE(dtime) AS datum,
    max.wmax - min.wmin AS Energy
FROM data m1
LEFT JOIN ( SELECT DATE(dtime) AS datum, MIN(W) AS wmin
            FROM data GROUP by DATE(dtime)
          ) min
        ON DATE(m1.dtime) = min.datum
LEFT JOIN ( SELECT DATE(dtime) AS datum, MAX(W) AS wmax
            FROM data GROUP by DATE(dtime)
          ) max
        ON DATE(m1.dtime) = max.datum
GROUP by DATE(m1.dtime)
LIMIT 20;
*/

-- Select data with PF calculation
/*
SELECT
    dtime, U, I, P, W, ROUND(100*p/(U*I),1) as pF
FROM data
ORDER BY dtime DESC
LIMIT 30;
*/


-- Select daily stat (last 30 days)
/*
SELECT
    DATE(dtime) AS 'date',
    MAX(W) - MIN(W) AS Energy,
    ROUND((MAX(W) - MIN(W))/24) AS AEph,
    MAX(P) AS `Max P`,
    ROUND(AVG(P)) AS `Avg P`,
    ROUND(100*AVG(p/(U*I)),1) as `Avg pF`
FROM data m1
WHERE devid='1'
GROUP by DATE(m1.dtime)
ORDER by dtime DESC
LIMIT 30;
*/


-- Select daily stat (w day/night) for the last month
/*
SELECT
    allday.date AS 'date',
    allday.Energy AS Energy,
    dayt.energy AS EnrgDay,
    allday.Energy - dayt.energy AS EnrgNight,
    ROUND( dayt.energy/allday.Energy * 100 ) AS PctDay,
    allday.MaxP AS PMax,
    allday.AvgP AS Pavg,
    dayt.AvgP AS PavgDay,
    dayt.AvgpF AS PFday,
    allday.AvgpF AS PF
FROM
    ( SELECT
	DATE(dtime) AS 'date',
	ROUND( (MAX(W) - MIN(W))/1000,2 ) AS Energy,
        ROUND(MAX(P)/1000,2) AS `MaxP`,
        ROUND(AVG(P)) AS `AvgP`,
	ROUND(100*AVG(p/(U*I)),1) as `AvgpF`
        FROM data
	WHERE devid='1'
	    AND dtime>CURDATE() - INTERVAL 1 MONTH
        GROUP by DATE(dtime)
     ) allday
LEFT JOIN
    ( SELECT
	DATE(dtime) AS 'date',
	ROUND( (MAX(W) - MIN(W))/1000,2 ) AS Energy,
        ROUND(AVG(P)) AS `AvgP`,
	ROUND(100*AVG(p/(U*I)),1) as `AvgpF`
      FROM data
	WHERE devid='1'
	    AND dtime>CURDATE() - INTERVAL 1 MONTH
	    AND HOUR(dtime) BETWEEN 7 AND 22
	GROUP by DATE(dtime)
    ) dayt
ON allday.date = dayt.date
ORDER by date DESC;
*/

-- Select year stat (last 12 month)
-- kW

SELECT
    allday.Month AS 'Month',
    allday.Energy AS Energy,
    dayt.energy AS EnrgDay,
    allday.Energy - dayt.energy AS EnrgNight,
    ROUND( dayt.energy/allday.Energy * 100 ) AS PctDay,
    allday.AEpD AS EApD,
    allday.MaxP AS PMax,
    allday.AvgP AS Pavg,
    dayt.AvgP   AS PavgDay,
    dayt.AvgpF  AS PFday,
    allday.AvgpF AS PF
FROM 
    ( SELECT
	MONTHNAME(dtime) AS 'Month',
	ROUND( (MAX(W) - MIN(W))/1000, 2 ) AS Energy,
	ROUND((MAX(W) - MIN(W))/ (DATEDIFF(MAX(dtime),MIN(dtime))+1)/1000, 2 ) AS AEpD,
        ROUND(MAX(P)/1000,2) AS `MaxP`,
        ROUND(AVG(P)/1000,2) AS `AvgP`,
	ROUND(100*AVG(p/(U*I))) as `AvgpF`
        FROM data
	WHERE devid='1' AND dtime>CURDATE() - INTERVAL 1 YEAR
        GROUP by MONTH(dtime)
	ORDER by dtime DESC
     ) allday
LEFT JOIN
    ( SELECT
      perday.Month,
      SUM(perday.Energy) AS Energy,
      ROUND(AVG(perday.P)/1000,2) AS `AvgP`,
      ROUND(100*AVG(perday.AvgpF)) as `AvgpF`
      FROM (
        SELECT
	    MONTHNAME(dtime) AS 'Month',
	    ROUND( (MAX(W) - MIN(W))/1000,2 ) AS Energy,
    	    AVG(P) AS `P`,
	    AVG(p/(U*I)) as `AvgpF`
        FROM data
	    WHERE devid='1' AND HOUR(dtime) BETWEEN 7 AND 22 AND dtime>CURDATE() - INTERVAL 1 YEAR
	    GROUP by DATE(dtime)
        ) perday
     GROUP by perday.Month
    ) dayt
ON allday.Month = dayt.Month;
-- ORDER by Month DESC;


/* no round
SELECT
    allday.Month AS 'Month',
    allday.Energy AS Energy,
    dayt.energy AS EnrgDay,
    allday.Energy - dayt.energy AS EnrgNight,
    ROUND( dayt.energy/allday.Energy * 100, 2 ) AS DayPct,
    allday.AEpD AS AEpD,
    allday.MaxP AS MaxP,
    dayt.AvgP AS avgDayP,
    allday.AvgP AS avgP,
    dayt.AvgpF AS dayPF,
    allday.AvgpF AS PF
FROM 
    ( SELECT
	MONTHNAME(dtime) AS 'Month',
	MAX(W) - MIN(W) AS Energy,
	(MAX(W) - MIN(W))/ DATEDIFF(MAX(dtime),MIN(dtime)) AS AEpD,
        MAX(P) AS `MaxP`,
        AVG(P) AS `AvgP`,
	ROUND(100*AVG(p/(U*I)),1) as `AvgpF`
        FROM data
	WHERE dtime>CURDATE() - INTERVAL 1 YEAR
        GROUP by MONTH(dtime)
     ) allday
LEFT JOIN
    ( SELECT
      perday.Month,
      SUM(perday.Energy) AS Energy,
      ROUND(AVG(perday.P)) AS `AvgP`,
      ROUND(100*AVG(perday.AvgpF),1) as `AvgpF`
      FROM (
        SELECT
	    MONTHNAME(dtime) AS 'Month',
	    MAX(W) - MIN(W) AS Energy,
    	    AVG(P) AS `P`,
	    AVG(p/(U*I)) as `AvgpF`
        FROM data
	    WHERE HOUR(dtime) BETWEEN 7 AND 22 AND dtime>CURDATE() - INTERVAL 1 YEAR
	    GROUP by DATE(dtime)
        ) perday
     GROUP by perday.Month
    ) dayt
ON allday.Month = dayt.Month
ORDER by Month DESC;
*/


/*
SELECT
    DATE(dtime) AS 'date',
    MAX(W) - MIN(W) AS Energy
FROM data m1
WHERE m1.dtime>CURDATE() - INTERVAL 1 WEEK
-- AND
--    HOUR(m1.dtime) BETWEEN 7 AND 22
GROUP by DATE(m1.dtime)
ORDER by dtime DESC;

*/