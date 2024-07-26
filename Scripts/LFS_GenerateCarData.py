#
# SliProSuperPro
# A Shift Light Indicator controller
# Copyright 2024 Fixfactory
#
# This file is part of SliProSuperPro.
#
# SliProSuperPro is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or any later version.
#
# SliProSuperPro is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with SliProSuperPro. If not, see <http://www.gnu.org/licenses/>.
#

from rauth import OAuth2Service
import json
import requests

# Import the secret credentials for authenticating to the LFS API.
# This should be a simple secret.py file defining client_id and client_secret
# Register your app at https://www.lfs.net/account/api/
# Uncheck SPA and use redirect URI https://www.lfs.net/
import secret

# Authenticate
print("Authenticating...")
accessTokenUrl = "https://id.lfs.net/oauth2/access_token"
baseUrl = "https://id.lfs.net/"

service = OAuth2Service(
    name="generatecardata",
    client_id=secret.client_id,
    client_secret=secret.client_secret,
    access_token_url=accessTokenUrl,
    authorize_url=accessTokenUrl,
    base_url=baseUrl,
)

data = {
    'grant_type': 'client_credentials',
}

session = service.get_auth_session(data=data, decoder=json.loads)
access_token = session.access_token

# Get the list of vehicle mods
print("Fetching vehicle mod list...")
endpointUrl = 'https://api.lfs.net/vehiclemod'
headers = {"Authorization": "Bearer " + access_token}
result = requests.get(endpointUrl, headers=headers)

vehicle_mods = result.json()

#with open("LFS_vehiclemods.json", "w") as write_file:
#    json.dump(vehiclemods, write_file, indent=4)

# Load existing car data file
print("Loading car data file...")
car_data = None
with open("..\\Data\\LiveForSpeed.CarData.json", "r") as read_file:
    car_data = json.load(read_file)

car_count = 1
car_total_count = len(vehicle_mods['data'])
car_data_mods = {}
for car in vehicle_mods['data']:
    print("Fetching car " + str(car_count) + "/" + str(car_total_count) + " " + car['id'] + " " + car['name'])
    car_count = car_count + 1
    endpointUrl = 'https://api.lfs.net/vehiclemod/' + car['id']
    headers = {"Authorization": "Bearer " + access_token}
    result = requests.get(endpointUrl, headers=headers)
    vehicle = result.json()

    #with open("LFS_vehiclemoddetails.json", "w") as write_file:
    #    json.dump(vehicle, write_file, indent=4)

    car_data_mods[car['id']] = {}
    car_data_mods[car['id']]['name'] = car['name']

    # The number of gear isn't specified. Shift ligths will blink at last gear.
    car_data_mods[car['id']]['finalGear'] = 0 

    # The rpm limit isn't specified. Use the maxPowerRpm as this is when we shift.
    car_data_mods[car['id']]['rpmLimit'] = vehicle['data']['vehicle']['maxPowerRpm']

    # We use maxTorqueRpm as an estimated downshift point. 
    car_data_mods[car['id']]['rpmDownshift'] = vehicle['data']['vehicle']['maxTorqueRpm']

    # We use maxPowerRpm as an estimated upshift point.
    car_data_mods[car['id']]['rpmUpshift'] = vehicle['data']['vehicle']['maxPowerRpm']

# Update the car data file
print("Writing car data file...")
car_data['cars']['mods'] = car_data_mods
with open("..\\Data\\LiveForSpeed.CarData.json", "w") as write_file:
    json.dump(car_data, write_file, indent=4)
    write_file.truncate()

print("Done")
