use smartbee_projeto;

select * from colmeia_tipo;

select * from colmeia_coleta;

SELECT (CONVERT (AVG(CAST(valor AS DECIMAL(5,2))), CHAR(255))) as valor FROM colmeia_coleta;


update colmeia_coleta set valor = '20' where id > 2;

select avg(valor) from colmeia_coleta;

SELECT * FROM Colmeia_coleta c where c.sensor_id=1 and c.colmeia_id=1 ORDER BY c.id DESC;

select * from colmeia;