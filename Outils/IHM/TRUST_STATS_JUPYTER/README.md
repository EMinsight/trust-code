```sh
   _________  _______             _______  _________
   \__   __/ (  ____ ) |\     /| (  ____ \ \__   __/
      ) (    | (    )| | )   ( | | (    \/    ) (
      | |    | (____)| | |   | | | (_____     | |
      | |    |     __) | |   | | (_____  )    | |
      | |    | (\ (    | |   | |       ) |    | |
      | |    | ) \ \__ | (___) | /\____) |    | |
      )_(    |/   \__/ (_______) \_______)    )_(


```


# TRUST_Outils

This code is a part of the open source TRUST platform (CEA DES/ISAS/DM2S/STMF/LGLS).

It provides a python library for TRUST users to
- Run validation files via jupyter lab and generate automatically a well documented report
- Post process the son files (probes) and plot the instantaneous/fluctuation evolution, time/space correlations, time/space energy spectra, ...

## Authors

- Victor Banon Garcia & Adrien Bruneton
- Morad Ben Tayeb & Élie Saikali

# Code organization

The parent package is called **trustutils**. Inside, you find three packages:
- **jupyter** : inside you find the **plot** and **run** modules
- **stats** : inside you find the **trust_statistical_tools** module
- **visitutils** : inside you find the **tools_for_visit** and **export_lata_base** modules

## Usage

Here are the steps to follow if the user wants to use the library from outside this package.

- Source the TRUST python environment

      source $TRUST_ROOT/env_for_python.sh

- Create a python script and do what you want to do. The modules can be loaded, for example, as follows

	   from trustutils.stats import SpatialPostProcessing, TemporalPostProcessing

	   from trustutils.jupyter import Graph

Examples of usage can be checked from the **tests** repository.

## Jupyter lab installation

Jupyter-lab can be easily installed as follows:

      source env_TRUST.sh

      conda install -c conda-forge jupyterlab ca-certificates=2020.10.14 conda=4.9.2 certifi=2020.6.20

## Python modules needed

In order to execute the Python code, the following modules are needed:

- **IPython** (Display images in jupyter lab)
- **Sphinx** (to generate documentation)
