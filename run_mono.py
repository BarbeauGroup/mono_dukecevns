import os
import json
import numpy as np

# Load config file
base_path = "jsonfiles/mono.json"
with open(base_path) as f:
    config = json.load(f)

nu_energy_arr = np.arange(1.0,56.0,1.0)

for i,nu_energy in enumerate(nu_energy_arr):
    # if i > 0: continue
    # Update config file
    config['flux']['energy'] = nu_energy
    with open('jsonfiles/mod.json', 'w') as f:
        json.dump(config, f)

    # Run the simulation
    os.system('./sns_rates mod')
    # Rename the output file and move them to the correct directory
    os.system('mv out/diff_rates_alliso-mod-Ge-klein.out out/klein/steps/diff_xs_nu_energy_{}.out'.format(nu_energy))
    os.system('mv out/diff_rates-mod-Ge-klein-integral.out out/klein/steps/xs_nu_energy_{}.txt'.format(nu_energy))
