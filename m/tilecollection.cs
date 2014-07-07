using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.Data;

namespace m
{
	public delegate void TileAddedEventHandler(Tile tile);
	public delegate void TileRemovedEventHandler(Tile tile);

	/// <summary>
	/// Summary description for TileCollection.
	/// </summary>
	public class TileCollection
	{
		private TilesDataSet m_dsTiles = new TilesDataSet();
		private ArrayList m_alsTiles = new ArrayList();
		public event TileAddedEventHandler TileAdded;
		public event TileRemovedEventHandler TileRemoved;

		public TileCollection() {
		}
		
		public bool Load(String strFile) {
			try {
				// At least make sure the file is valid before anything
				TilesDataSet dsTiles = new TilesDataSet();
				dsTiles.ReadXml(strFile);

				// Remove current tiles
				while (Count != 0)
					RemoveTile(this[0]);
				
				// Use this TileDataSet and load tiles
				m_dsTiles = dsTiles;
				foreach (TilesDataSet.TilesRow row in m_dsTiles.Tiles) {
					Tile tile = new Tile(row);
					m_alsTiles.Add(tile);
					OnTileAdded(tile);
				}
				return true;
			} catch {
				return false;
			}
		}

		public bool Save(String strFile) {
			try {
				foreach (Tile tile in m_alsTiles)
					tile.Save();
				m_dsTiles.WriteXml(strFile);
				return true;
			} catch {
				return false;
			}
		}

		public Tile NewTile(String strFileBitmap) {
			TilesDataSet.TilesRow row = m_dsTiles.Tiles.NewTilesRow();
			Tile tile = new Tile(row);
			if (tile.Import(strFileBitmap)) {
				m_dsTiles.Tiles.AddTilesRow(row);
				m_alsTiles.Add(tile);
				OnTileAdded(tile);
				return tile;
			}
			return null;
		}

		public void RemoveTile(Tile tile) {
			if (!m_alsTiles.Contains(tile))
				return;
			m_dsTiles.Tiles.RemoveTilesRow(tile.Row);
			m_alsTiles.Remove(tile);
			OnTileRemoved(tile);
		}

		public Tile FindTile(int cookie) {
			foreach (Tile tile in m_alsTiles) {
				if (tile.Cookie == cookie)
					return tile;
			}
			return null;
		}

		// Event firing

		private void OnTileRemoved(Tile tile) {
			if (TileRemoved != null)
				TileRemoved(tile);
		}

		private void OnTileAdded(Tile tile) {
			if (TileAdded != null)
				TileAdded(tile);
		}

		// Properties

		public Tile this[int index] {
			get {
				return (Tile)m_alsTiles[index];
			}
		}

		public int Count {
			get {
				return m_alsTiles.Count;
			}
		}

		// Enumeration support

		public TileEnumerator GetEnumerator() {
			return new TileEnumerator(this);
		}

		public class TileEnumerator {
			private TileCollection m_tcol;
			private int m_pos = -1;

			public TileEnumerator(TileCollection tcol) {
				m_tcol = tcol;
			}

			public bool MoveNext() {
				if (m_pos < m_tcol.m_alsTiles.Count - 1) {
					m_pos++;
					return true;
				}
				return false;
			}

			public void Reset() {
				m_pos = -1;
			}

			public Tile Current {
				get {
					return (Tile)m_tcol.m_alsTiles[m_pos];
				}
			}
		}
	}
}
