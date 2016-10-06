#!/bin/bash
##################################################################################
#                            BIENVENUE SUR CALLISTO !                            #
#                                                                                #
# * Documentation : evince /cm/shared/docs/callisto.pdf                          #
# * Remarque : Les noeuds des partitions slim, large et fat sont interconnectes  #
#              par un reseau InfiniBand PSM alors que ceux des partitions eris   #
#              et pluton s'appuient sur des equipements Mellanox.                #
#              Les jobs qui ont recours au module OpenMPI peuvent s'executer sur #
#              les deux partitions. Les utilisateurs de MVAPICH2 devront quant   #
#              a eux utiliser des modules specifiques a chaque type d'IB.        #
# * Astuce : Pour diminuer vos temps d'attente, il est possible de specifier     #
#            plusieurs partitions au scheduler, par exemple "-p slim,large,eris".#
#            Vos jobs seront ainsi executes sur la premiere partition a disposer #
#            des ressources necessaires.                                         #
#                                                                                #
##################################################################################

##################################
# Variables for configure script #
##################################
define_modules_config()
{
   env=$TRUST_ROOT/env/machine.env
   # Load modules
   module=slurm
   echo "# Module $module detected and loaded on $HOST."
   # Initialisation de l environnement module $MODULE_PATH 
   echo "source /cm/local/apps/environment-modules/3.2.10/init/bash" >> $env
   echo "module load $module 1>/dev/null" >> $env
   #
   #GNU gcc/4.4.6 gcc/4.8.1 gcc/4.9.0 gcc/4.9.3 gcc/5.2.0 (4.4.7 default)
   #intel="gcc/5.2.0"
   #OPENMPI module openmpi/gcc/64/1.8 openmpi/gcc/64/1.8.3 openmpi/gcc/64/1.8.4 
   # Aero_192: 3.7s
   module="openmpi/gcc/64/1.8.3"
   #module="openmpi/gcc/64/1.8.4"
   #module="openmpi/gcc/64/1.10.0"
   #module="openmpi/gcc/64/1.10.1"
   #module="openmpi/gcc/64/1.10.3"
   #module="$intel openmpi/gcc/64/1.8.3"
   #
   echo "# Module $module detected and loaded on $HOST."
   echo "module unload mpich openmpi mvapich mvapich2 intel/compiler intel/mkl intel/tbb gcc 1>/dev/null" >> $env
   echo "module load $module 1>/dev/null" >> $env     
   . $env
   # Creation wrapper qstat -> squeue
   echo "#!/bin/bash
squeue" > $TRUST_ROOT/bin/qstat
   chmod +x $TRUST_ROOT/bin/qstat
}

##############################
# Variables for trust script #
##############################
define_soumission_batch()
{
   soumission=2 && [ "$prod" = 1 ] && soumission=1
   # sinfo :
   #PARTITION AVAIL  TIMELIMIT  NODES  STATE
   #slim*        up   infinite     37    mix
   #fat          up   infinite      1    mix
   #large        up   infinite     13    mix
   #eris         up   infinite     30  alloc
   #pluton       up   infinite      4    mix
   queue=slim && [ "$bigmem" = 1 ] && queue=large && soumission=1
   # sacctmgr list qos format=Name,Priority,MaxSubmit,MaxWall,MaxNodes :      
   # Name     Priority MaxSubmit     MaxWall 
   #------- ---------- --------- -----------
   # normal         20            2-00:00:00
   #   test         40         2    01:00:00
   #   long         10           14-00:00:00
   # project_r+     40           31-00:00:00
   cpu=30 && [ "$prod" = 1 ] && cpu=1440 # 30 minutes or 1 day
   ntasks=20 # 20 cores per node for slim or large queue (24 for fat, 8 for eris and 12 for pluton)
   if [ "$prod" = 1 ] || [ $NB_PROCS -gt $ntasks ]
   then
      if [ "`echo $NB_PROCS | awk -v n=$ntasks '{print $1%n}'`" != 0 ]
      then
         echo "=================================================================================================================="
         echo "Warning: the allocated nodes of $ntasks cores will not be shared with other jobs (--exclusive option used)"
	 echo "so please try to fill the allocated nodes by partitioning your mesh with multiple of $ntasks on $queue partition."
	 echo "=================================================================================================================="
      fi
      node=1
      qos=normal
      [ "$prod" = 1 ] && cpu=2880 # 2 days
   else
      qos=test
      [ "$prod" = 1 ] && cpu=60 # 1 hour
   fi
   #if [ "`basename $Mpirun`" = srun ]
   #then
   #   # Slurm srun support
   #   mpirun="srun -n $NB_PROCS"
   #else
   #   mpirun="mpirun -np $NB_PROCS"
   #fi
   # 01/07/2014: Regression when using mpirun with openmpi (very bad performance *3) so:
   # srun for everyone (even if it is slightly slower, binding ?)
   #mpirun="srun --cpu_bind=verbose,cores -n $NB_PROCS"
   # 18/08/2015: avec openmpi 1.8.4 srun ne passe pas... donc on repasse a mpirun -np
   #mpirun="mpirun -np $NB_PROCS"
   # retour en arriere openmpi 1.8.3
   mpirun="srun -n \$SLURM_NTASKS"
   sub=SLURM
}
