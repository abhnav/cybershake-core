package mapping;

// Generated Aug 9, 2007 9:53:47 PM by Hibernate Tools 3.2.0.b9

import java.util.HashSet;
import java.util.Set;

/**
 * ErfIds generated by hbm2java
 */
public class ErfIds implements java.io.Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 4160463905166788058L;

	private int erfId;

	private String erfName;

	private String erfDescription;

	private Set<Ruptures> ruptureses = new HashSet<Ruptures>(0);

	private Set<ErfMetadata> erfMetadatas = new HashSet<ErfMetadata>(0);

	private Set<CyberShakeSiteRegions> cyberShakeSiteRegionses = new HashSet<CyberShakeSiteRegions>(
			0);

	private Set<PeakAmplitudes> peakAmplitudeses = new HashSet<PeakAmplitudes>(
			0);

	private Set<Points> pointses = new HashSet<Points>(0);

	private Set<RuptureVariations> ruptureVariationses = new HashSet<RuptureVariations>(
			0);

	private Set<RuptureVariationScenarioIds> ruptureVariationScenarioIdses = new HashSet<RuptureVariationScenarioIds>(
			0);

	private Set<SgtVariationIds> sgtVariationIdses = new HashSet<SgtVariationIds>(
			0);

	private Set<CyberShakeSiteRuptures> cyberShakeSiteRuptureses = new HashSet<CyberShakeSiteRuptures>(
			0);

	public ErfIds() {
	}

	public ErfIds(int erfId, String erfName) {
		this.erfId = erfId;
		this.erfName = erfName;
	}

	public ErfIds(int erfId, String erfName, String erfDescription,
			Set<Ruptures> ruptureses, Set<ErfMetadata> erfMetadatas,
			Set<CyberShakeSiteRegions> cyberShakeSiteRegionses,
			Set<PeakAmplitudes> peakAmplitudeses, Set<Points> pointses,
			Set<RuptureVariations> ruptureVariationses,
			Set<RuptureVariationScenarioIds> ruptureVariationScenarioIdses,
			Set<SgtVariationIds> sgtVariationIdses,
			Set<CyberShakeSiteRuptures> cyberShakeSiteRuptureses) {
		this.erfId = erfId;
		this.erfName = erfName;
		this.erfDescription = erfDescription;
		this.ruptureses = ruptureses;
		this.erfMetadatas = erfMetadatas;
		this.cyberShakeSiteRegionses = cyberShakeSiteRegionses;
		this.peakAmplitudeses = peakAmplitudeses;
		this.pointses = pointses;
		this.ruptureVariationses = ruptureVariationses;
		this.ruptureVariationScenarioIdses = ruptureVariationScenarioIdses;
		this.sgtVariationIdses = sgtVariationIdses;
		this.cyberShakeSiteRuptureses = cyberShakeSiteRuptureses;
	}

	public int getErfId() {
		return this.erfId;
	}

	public void setErfId(int erfId) {
		this.erfId = erfId;
	}

	public String getErfName() {
		return this.erfName;
	}

	public void setErfName(String erfName) {
		this.erfName = erfName;
	}

	public String getErfDescription() {
		return this.erfDescription;
	}

	public void setErfDescription(String erfDescription) {
		this.erfDescription = erfDescription;
	}

	public Set<Ruptures> getRuptureses() {
		return this.ruptureses;
	}

	public void setRuptureses(Set<Ruptures> ruptureses) {
		this.ruptureses = ruptureses;
	}

	public Set<ErfMetadata> getErfMetadatas() {
		return this.erfMetadatas;
	}

	public void setErfMetadatas(Set<ErfMetadata> erfMetadatas) {
		this.erfMetadatas = erfMetadatas;
	}

	public Set<CyberShakeSiteRegions> getCyberShakeSiteRegionses() {
		return this.cyberShakeSiteRegionses;
	}

	public void setCyberShakeSiteRegionses(
			Set<CyberShakeSiteRegions> cyberShakeSiteRegionses) {
		this.cyberShakeSiteRegionses = cyberShakeSiteRegionses;
	}

	public Set<PeakAmplitudes> getPeakAmplitudeses() {
		return this.peakAmplitudeses;
	}

	public void setPeakAmplitudeses(Set<PeakAmplitudes> peakAmplitudeses) {
		this.peakAmplitudeses = peakAmplitudeses;
	}

	public Set<Points> getPointses() {
		return this.pointses;
	}

	public void setPointses(Set<Points> pointses) {
		this.pointses = pointses;
	}

	public Set<RuptureVariations> getRuptureVariationses() {
		return this.ruptureVariationses;
	}

	public void setRuptureVariationses(
			Set<RuptureVariations> ruptureVariationses) {
		this.ruptureVariationses = ruptureVariationses;
	}

	public Set<RuptureVariationScenarioIds> getRuptureVariationScenarioIdses() {
		return this.ruptureVariationScenarioIdses;
	}

	public void setRuptureVariationScenarioIdses(
			Set<RuptureVariationScenarioIds> ruptureVariationScenarioIdses) {
		this.ruptureVariationScenarioIdses = ruptureVariationScenarioIdses;
	}

	public Set<SgtVariationIds> getSgtVariationIdses() {
		return this.sgtVariationIdses;
	}

	public void setSgtVariationIdses(Set<SgtVariationIds> sgtVariationIdses) {
		this.sgtVariationIdses = sgtVariationIdses;
	}

	public Set<CyberShakeSiteRuptures> getCyberShakeSiteRuptureses() {
		return this.cyberShakeSiteRuptureses;
	}

	public void setCyberShakeSiteRuptureses(
			Set<CyberShakeSiteRuptures> cyberShakeSiteRuptureses) {
		this.cyberShakeSiteRuptureses = cyberShakeSiteRuptureses;
	}

}
