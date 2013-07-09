mysql -u rucio -p -D rucio -s -N -e 'set time_zone="+00:00"; select token from tokens where expired_at>now();'
