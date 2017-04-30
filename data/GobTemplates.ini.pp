#include <res.h>

// MoveRate (in wc/upd) - WCoord moved per update. MUST BE LESS THAN 64!
// FiringRate (in msecs) - delay between firings
// TimeToBuild (in secs) - amount of time required to build this structure/unit. MUST BE LESS THAN 1000!
// ArmorStrength (in hitpoints) - when this falls to 0 the structure/unit dies
// Infantry/Vehicle/StructureDamage (in hitpoints) - amount of hitpoints subtracted from target class when hit
// SightRange (in tiles) - distance the structure/unit can 'see'
// FiringRange (in tiles) - distance the structure/unit can fire
// Cost - # of credits consumed by creating this unit

// Infantry Units

[kgtShortRangeInfantry]
Name=Guard
LongName=Security Guard
Cost=150
CostMP=100
MoveRate=18
ArmorStrength=39
InfantryDamage=9
InfantryDamageMP=12
VehicleDamage=3
StructureDamage=3
FiringRate=750
FiringRange=2
TimeToBuild=16
TimeToBuildMP=10
UIBoundsLeft=-6
UIBoundsTop=-7
UIBoundsRight=7
UIBoundsBottom=7
SortOffset=5
Animation=sri.anir
Description=Cheap and cheerful. Limited firepower and defensive capability.

[kgtLongRangeInfantry]
Name=Trooper
LongName=Rocket Trooper
Cost=300
MoveRate=11
ArmorStrength=50
InfantryDamage=6
VehicleDamage=17
StructureDamage=6
FiringRate=1600
FiringRange=3
TimeToBuild=16
UIBoundsLeft=-6
UIBoundsTop=-7
UIBoundsRight=7
UIBoundsBottom=7
SortOffset=5
Animation=lri.anir
Description=For those hard to reach places. Rockets are particularly effective against armored vehicles.

// Corporate Raider's StructureDamage and FiringRate are set to match kfxDamagePerSecMax
// so it will show 8 structure icons for its damage in the build form

[kgtTakeoverSpecialist]
Name=Raider
LongName=Corporate Raider
Cost=550
MoveRate=10
MoveRateMP=16
ArmorStrength=50
InfantryDamage=0
VehicleDamage=0
StructureDamage=16
FiringRate=1000
FiringRange=1
TimeToBuild=16
UIBoundsLeft=-6
UIBoundsTop=-7
UIBoundsRight=7
UIBoundsBottom=7
SortOffset=5
Animation=spi.anir
Description=Send the Raider to take over ememy buildings. Keep in mind he has no conventional weapons and little armor!

[kgtAndy]
Name=Andy
LongName=Andy
Cost=550
MoveRate=22
ArmorStrength=100
InfantryDamage=56
VehicleDamage=24
StructureDamage=8
FiringRate=2300
FiringRange=4
TimeToBuild=16
UIBoundsLeft=-6
UIBoundsTop=-7
UIBoundsRight=7
UIBoundsBottom=7
SortOffset=5
Animation=andy.anir
Description=Long range scout and sniper

[kgtFox]
Name=Fox
LongName=Fox
Cost=550
MoveRate=18
ArmorStrength=80
InfantryDamage=56
VehicleDamage=24
StructureDamage=8
FiringRate=2300
FiringRange=4
TimeToBuild=16
UIBoundsLeft=-6
UIBoundsTop=-7
UIBoundsRight=7
UIBoundsBottom=7
SortOffset=5
Animation=andy.anir
Description=Bad guy

// Mechanical Units

[kgtMachineGunVehicle]
Name=Eagle
LongName=SR-98 Eagle
Cost=300
MoveRate=24
ArmorStrength=75
InfantryDamage=8
VehicleDamage=4
StructureDamage=4
FiringRate=1000
FiringRange=3
TimeToBuild=16
UIBoundsLeft=-10
UIBoundsTop=-10
UIBoundsRight=10
UIBoundsBottom=11
SortOffset=7
Animation=tmac.anir
Description=Fast but not particularly powerful or well armored. Makes a nice scout.

[kgtLightTank]
Name=Broadsword
LongName=T-29 Broadsword
Cost=450
MoveRate=16
ArmorStrength=125
InfantryDamage=6
VehicleDamage=9
StructureDamage=15
FiringRate=1500
FiringRange=3
TimeToBuild=16
UIBoundsLeft=-10
UIBoundsTop=-10
UIBoundsRight=10
UIBoundsBottom=11
SortOffset=7
Animation=ltank.anir
Description=Equipped with a fine mortar cannon. Increased damage against buildings.

[kgtRocketVehicle]
Name=Hydra
LongName=M-18 Hydra
Cost=450
CostMP=550
MoveRate=14
ArmorStrength=100
ArmorStrengthMP=85
InfantryDamage=5
VehicleDamage=20
StructureDamage=8
FiringRate=1250
FiringRange=3
TimeToBuild=16
UIBoundsLeft=-10
UIBoundsTop=-10
UIBoundsRight=10
UIBoundsBottom=11
SortOffset=7
Animation=troc.anir
Description=Well armored and equipped with high-explosive rockets that really pack a punch against vehicles.

[kgtMediumTank]
Name=Liberator
LongName=T-33 Liberator
Cost=600
MoveRate=15
ArmorStrength=150
InfantryDamage=16
VehicleDamage=6
StructureDamage=4
FiringRate=1000
FiringRange=3
TimeToBuild=16
UIBoundsLeft=-10
UIBoundsTop=-10
UIBoundsRight=10
UIBoundsBottom=11
SortOffset=7
Animation=mtank.anir
Description=This tank's dual cannons fire fragmentation grenades that do serious damage to enemy infantry.

[kgtArtillery]
Name=Cyclops
LongName=A-3 Cyclops
Cost=500
MoveRate=10
ArmorStrength=50
InfantryDamage=7
VehicleDamage=24
StructureDamage=48
FiringRate=3000
FiringRange=4
TimeToBuild=16
UIBoundsLeft=-12
UIBoundsTop=-12
UIBoundsRight=12
UIBoundsBottom=13
SortOffset=7
Animation=artillery.anir
Description=Long range of fire. Slow moving. Packs a punch but is not accurate against smaller units.

[kgtGalaxMiner]
Name=Bullpup
LongName=G-4 Bullpup
Cost=600
MoveRate=14
ArmorStrength=200
FiringRate=0
FiringRange=0
TimeToBuild=16
UIBoundsLeft=-10
UIBoundsTop=-10
UIBoundsRight=10
UIBoundsBottom=11
SortOffset=7
Animation=miner.anir
Menu=kidfMinerMenu
Description=Increase your mining rate with additional Bullpups. It has no weapons but can take a lot of punishment.

[kgtMobileHeadquarters]
Name=Dominion
LongName=H-7 Dominion
Cost=1200
MoveRate=12
ArmorStrength=200
FiringRate=0
FiringRange=0
TimeToBuild=16
UIBoundsLeft=-10
UIBoundsTop=-10
UIBoundsRight=10
UIBoundsBottom=11
SortOffset=7
Animation=mobilehq.anir
Menu=kidfMobileHqMenu
Description=Not happy where you are? Start a new base at a remote location with this Mobile Headquarters.

// Structures

[kgtHumanResourceCenter]
Name=H.R.C.
LongName=Human Resource Center (HRC)
TimeToBuild=15
Cost=1000
PowerDemand=10
ArmorStrength=250
UIBoundsLeft=-1
UIBoundsTop=-1
UIBoundsRight=49
UIBoundsBottom=33
SortOffset=29
Animation=hrc.anir
TileDimensions=3,2
ReserveDimensions=3,3
Menu=kidfHrcMenu
BuildRate=2
UpgradeCost=750
Description=Use this building to recruit new personnel. Upgrade the HRC to gain access to more advanced units.

[kgtProcessor]
Name=Processor
LongName=Galaxite Processor
TimeToBuild=15
Cost=1500
PowerDemand=10
ArmorStrength=350
UIBoundsLeft=0
UIBoundsTop=-1
UIBoundsRight=48
UIBoundsBottom=32
SortOffset=32
Animation=proc.anir
TileDimensions=3,2
ReserveDimensions=3,3
Description=Earth HQ provides credit for processed Galaxite. This building comes complete with one mining vehicle (Bullpup)! A Processor can store 3000 credits worth of Galaxite.

[kgtReactor]
Name=Generator
LongName=Power Generator
TimeToBuild=10
Cost=750
PowerSupply=40
ArmorStrength=200
UIBoundsLeft=1
UIBoundsTop=3
UIBoundsRight=33
UIBoundsBottom=31
SortOffset=30
Animation=reactor.anir
TileDimensions=2,2
Description=Most buildings will not operate without sufficient power. Keep an eye on your power supply and demand at all times.

[kgtHeadquarters]
Name=H.Q.
LongName=Headquarters (HQ)
TimeToBuild=15
Cost=750
ArmorStrength=500
UIBoundsLeft=0
UIBoundsTop=0
UIBoundsRight=48
UIBoundsBottom=32
SortOffset=30
Animation=hq.anir
TileDimensions=3,2
Menu=kidfHqMenu
BuildRate=100

[kgtRadar]
Name=S.C.
LongName=Surveillance Center (SC)
TimeToBuild=15
Cost=750
PowerDemand=15
ArmorStrength=250
UIBoundsLeft=-1
UIBoundsTop=-1
UIBoundsRight=33
UIBoundsBottom=33
SortOffset=30
Animation=radar.anir
TileDimensions=2,2
Description=A Surveillance Center provides guidance control for Gun and Rocket Towers and is required to build them.

[kgtResearchCenter]
Name=R&D Center
LongName=Research & Development Center
TimeToBuild=15
Cost=750
PowerDemand=15
ArmorStrength=250
UIBoundsLeft=-1
UIBoundsTop=-1
UIBoundsRight=33
UIBoundsBottom=33
SortOffset=30
Animation=research.anir
TileDimensions=2,2
Menu=kidfResearchMenu
Description=Invest in an R&D Center and its scientists will work hard to produce technological advances to aid you in your operations.

[kgtVehicleTransportStation]
Name=V.T.S.
LongName=Vehicle Transport Station (VTS)
TimeToBuild=15
Cost=1250
CostMP=2000
PowerDemand=10
ArmorStrength=300
UIBoundsLeft=-1
UIBoundsTop=-1
UIBoundsRight=49
UIBoundsBottom=33
SortOffset=30
Animation=vts.anir
TileDimensions=3,2
ReserveDimensions=3,3
Menu=kidfVtsMenu
BuildRate=10
UpgradeCost=1000
Description=Use this building to order new vehicles. Upgrade the VTS to gain access to more advanced units.

[kgtWarehouse]
Name=Warehouse
LongName=Galaxite Storage Warehouse
TimeToBuild=15
Cost=750
PowerDemand=7
ArmorStrength=200
UIBoundsLeft=0
UIBoundsTop=0
UIBoundsRight=33
UIBoundsBottom=33
SortOffset=30
Animation=warehouse.anir
TileDimensions=2,2
Description=Add Warehouses to increase processed Galaxite inventory capacity. Each Warehouse can store up to 5000 credits worth of Galaxite.

[kgtReplicator]
Name=Replicator
LongName=Replicator
TimeToBuild=15
Cost=1000
PowerDemand=0
ArmorStrength=300
UIBoundsLeft=3
UIBoundsTop=0
UIBoundsRight=77
UIBoundsBottom=56
SortOffset=84
Animation=replicator.anir
TileDimensions=5,4
ReserveDimensions=5,4
BuildRate=2
UpgradeCost=400
Description=You're not supposed to be able to build this!

// Towers

[kgtMachineGunTower]
Name=Gatling
LongName=Gatling Tower
TimeToBuild=10
Cost=750
PowerDemand=10
ArmorStrength=150
InfantryDamage=16
VehicleDamage=5
StructureDamage=6
FiringRate=1333
FiringRange=3
UIBoundsLeft=0
UIBoundsTop=0
UIBoundsRight=16
UIBoundsBottom=16
SortOffset=30
Animation=mtower.anir
TileDimensions=1,1
Description=Defend your base from unwanted visits by the competition. Gatling Towers are particularly effective against troops. NOTE: a Surveillance Center is required to guide their automatic targeting systems.

[kgtRocketTower]
Name=Rocket Twr
LongName=Rocket Tower
TimeToBuild=10
Cost=850
PowerDemand=10
ArmorStrength=150
InfantryDamage=6
VehicleDamage=18
StructureDamage=6
FiringRate=1500
FiringRange=3
UIBoundsLeft=0
UIBoundsTop=0
UIBoundsRight=16
UIBoundsBottom=16
SortOffset=30
Animation=rtower.anir
TileDimensions=1,1
Description=Rocket Towers are particularly effective against armored vehicles. NOTE: a Surveillance Center is required to guide their automatic targeting systems.
