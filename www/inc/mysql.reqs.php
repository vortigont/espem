<?php
//MySQL Requests


// Select year stat (last 12 month)
$sql['ystat'] = <<<SQL
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
    allday.AvgpF AS PFavg
FROM
    ( SELECT
        MONTHNAME(dtime) AS 'Month',
        ROUND( (MAX(W) - MIN(W))/1000, 2 ) AS Energy,
        ROUND((MAX(W) - MIN(W))/ (DATEDIFF(MAX(dtime),MIN(dtime))+1)/1000, 2 ) AS AEpD,
        ROUND(MAX(P)/1000,2) AS `MaxP`,
        ROUND(AVG(P)/1000,2) AS `AvgP`,
        ROUND(100*AVG(p/(U*I))) as `AvgpF`
        FROM data
        WHERE devid='$devid' AND dtime>CURDATE() - INTERVAL 1 YEAR
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
            WHERE devid='$devid' AND HOUR(dtime) BETWEEN $dtsh AND $dteh AND dtime>CURDATE() - INTERVAL 1 YEAR
            GROUP by DATE(dtime)
        ) perday
     GROUP by perday.Month
    ) dayt
ON allday.Month = dayt.Month
SQL;

//-- Select daily stat (w day/night) for the last month
$sql['mstat'] = <<<SQL
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
    allday.AvgpF AS PFavg
FROM
    ( SELECT
        DATE(dtime) AS 'date',
        ROUND( (MAX(W) - MIN(W))/1000,2 ) AS Energy,
        ROUND(MAX(P)/1000,2) AS `MaxP`,
        ROUND(AVG(P)/1000,2) AS `AvgP`,
        ROUND(100*AVG(p/(U*I)),1) as `AvgpF`
        FROM data
        WHERE devid='$devid'
            AND dtime>CURDATE() - INTERVAL 1 MONTH
        GROUP by DATE(dtime)
     ) allday
LEFT JOIN
    ( SELECT
        DATE(dtime) AS 'date',
        ROUND( (MAX(W) - MIN(W))/1000,2 ) AS Energy,
        ROUND(AVG(P)/1000,2) AS `AvgP`,
        ROUND(100*AVG(p/(U*I)),1) as `AvgpF`
      FROM data
        WHERE devid='$devid'
            AND dtime>CURDATE() - INTERVAL 1 MONTH
            AND HOUR(dtime) BETWEEN $dtsh AND $dteh
        GROUP by DATE(dtime)
    ) dayt
ON allday.date = dayt.date
ORDER by date DESC
SQL;

//-- Select last recorded values
$sql['last'] =
"SELECT
    dtime, U, I, P, W, ROUND(100*P/(U*I)) as PF
FROM data
WHERE devid='$devid'
ORDER BY dtime DESC
LIMIT 1";




// $sql['some'] = <<<SQL
//SQL;


?>