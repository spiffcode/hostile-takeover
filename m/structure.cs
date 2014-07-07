using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.Serialization;
using SpiffLib;
using System.Text.RegularExpressions;

namespace m {
	// Wrappers for graceful future versioning
	[Serializable]
	public class RocketTower : Structure {
		public RocketTower(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public RocketTower(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public RocketTower(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class MachineGunTower : Structure {
		public MachineGunTower(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public MachineGunTower(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public MachineGunTower(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}
	}

	[Serializable]
	public class Warehouse : Structure {
		public Warehouse(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public Warehouse(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Warehouse(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 2;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class VehicleTransportStation : Structure {
		public VehicleTransportStation(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public VehicleTransportStation(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public VehicleTransportStation(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 3;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class Radar : Structure {
		public Radar(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public Radar(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Radar(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 2;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class ResearchCenter : Structure {
		public ResearchCenter(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public ResearchCenter(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public ResearchCenter(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 2;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class Headquarters : Structure {
		public Headquarters(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public Headquarters(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Headquarters(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 3;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class Reactor : Structure {
		public Reactor(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public Reactor(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Reactor(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 2;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class HumanResourceCenter : Structure {
		public HumanResourceCenter(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public HumanResourceCenter(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public HumanResourceCenter(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 3;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class Processor : Structure {
		public Processor(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public Processor(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Processor(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 3;
			}
		}

		public override int cty {
			get {
				return 2;
			}
		}
	}

	[Serializable]
	public class Replicator : Structure {
		public Replicator(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public Replicator(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Replicator(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override int ctx {
			get {
				return 5;
			}
		}

		public override int cty {
			get {
				return 4;
			}
		}
	}

	[Serializable]
	public class Activator : Unit {
		public Activator(Side side, int tx, int ty) : base (side, tx, ty) {
		}

		public Activator(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Activator(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override Point GetTileOrigin(Size sizTile) {
			// Activators have an origin at the top left of a tile

			return new Point(0, 0);
		}
	}

	[Serializable]
	public class Structure : Unit {
		public Structure(Side side, int tx, int ty) : base(side, tx, ty) {
		}

		public Structure(string strName, string strValue, int txOrigin, int tyOrigin) :
				base(strName, strValue, txOrigin, tyOrigin) {
		}

		public Structure(SerializationInfo info, StreamingContext ctx) : base(info, ctx) {
		}

		public override Point GetTileOrigin(Size sizTile) {
			// Structs have an origin at the top left of a tile

			return new Point(0, 0);
		}
	}
}
