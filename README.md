# Proyecto de programación orientada al rendimiento


## Ejecución del proyecto
Para ello compilar el proyecto ejecutando 

`sbatch build.sh`

Una vez copilado generara una carpeta build donde se podrá ejecutar el programa principal fluid

`build/fluid/fluid iteraciones input.fld output.fld`

Para ejecutar los utests se cuenta con el script runutest.sh

`sbatch runutest.sh`

Para ejecutar ftests se ha realizado un script `ftest/testFluid.sh` (el cual recibe dos parámetros: la ruta de fluid.sh y la ruta de ftest.sh) y se ejecuta con

`sbatch runftest.sh`

Además se incorporan algunos scripts que hemos utilizado para medir el rendimiento como

Para realizar las estadísticas de rendimiento de large.fld `sbatch perfeslarge.sh` 

Para realizar las estadísticas de rendimiento de small.fld `sbatch perfessmall.sh` 

Para realizar la medición de la energía `sbatch runenergy.sh` 
