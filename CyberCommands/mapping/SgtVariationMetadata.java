package mapping;

// Generated Aug 9, 2007 9:53:47 PM by Hibernate Tools 3.2.0.b9

/**
 * SgtVariationMetadata generated by hbm2java
 */
public class SgtVariationMetadata implements java.io.Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1639938845227854923L;

	private SgtVariationMetadataId id;

	private SgtVariationIds sgtVariationIds;

	private String sgtVariationAttrType;

	private String sgtVariationAttrUnits;

	public SgtVariationMetadata() {
	}

	public SgtVariationMetadata(SgtVariationMetadataId id,
			SgtVariationIds sgtVariationIds, String sgtVariationAttrType) {
		this.id = id;
		this.sgtVariationIds = sgtVariationIds;
		this.sgtVariationAttrType = sgtVariationAttrType;
	}

	public SgtVariationMetadata(SgtVariationMetadataId id,
			SgtVariationIds sgtVariationIds, String sgtVariationAttrType,
			String sgtVariationAttrUnits) {
		this.id = id;
		this.sgtVariationIds = sgtVariationIds;
		this.sgtVariationAttrType = sgtVariationAttrType;
		this.sgtVariationAttrUnits = sgtVariationAttrUnits;
	}

	public SgtVariationMetadataId getId() {
		return this.id;
	}

	public void setId(SgtVariationMetadataId id) {
		this.id = id;
	}

	public SgtVariationIds getSgtVariationIds() {
		return this.sgtVariationIds;
	}

	public void setSgtVariationIds(SgtVariationIds sgtVariationIds) {
		this.sgtVariationIds = sgtVariationIds;
	}

	public String getSgtVariationAttrType() {
		return this.sgtVariationAttrType;
	}

	public void setSgtVariationAttrType(String sgtVariationAttrType) {
		this.sgtVariationAttrType = sgtVariationAttrType;
	}

	public String getSgtVariationAttrUnits() {
		return this.sgtVariationAttrUnits;
	}

	public void setSgtVariationAttrUnits(String sgtVariationAttrUnits) {
		this.sgtVariationAttrUnits = sgtVariationAttrUnits;
	}

}