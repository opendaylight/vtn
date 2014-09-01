/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.validation.logical;

import java.math.BigInteger;

import com.google.gson.JsonObject;
import org.opendaylight.vtn.core.util.Logger;
import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;
import org.opendaylight.vtn.javaapi.constants.VtnServiceJsonConsts;
import org.opendaylight.vtn.javaapi.exception.VtnServiceException;
import org.opendaylight.vtn.javaapi.ipc.enums.UncJavaAPIErrorCode;
import org.opendaylight.vtn.javaapi.resources.AbstractResource;
import org.opendaylight.vtn.javaapi.resources.logical.PolicingProfilesEntriesResource;
import org.opendaylight.vtn.javaapi.resources.logical.PolicingProfilesEntryResource;
import org.opendaylight.vtn.javaapi.validation.CommonValidator;
import org.opendaylight.vtn.javaapi.validation.VtnServiceValidator;

/**
 * The Class PolicingProfilesEntriesResourceValidator validates request Json
 * object for PolicingProfilesEntriesResource and PolicingProfilesEntryResource
 * API.
 */
public class PolicingProfilesEntriesResourceValidation extends
		VtnServiceValidator {
	private static final Logger LOG = Logger
			.getLogger(PolicingProfilesEntriesResourceValidation.class
					.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Policing Profiles Entries Resource Validationr.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public PolicingProfilesEntriesResourceValidation(
			final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate uri parameters for PolicingProfilesEntriesResource
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start PolicingProfilesEntriesValidation#validateUri()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.URI
				+ VtnServiceJsonConsts.PROFILE_NAME);
		if (resource instanceof PolicingProfilesEntriesResource
				&& ((PolicingProfilesEntriesResource) resource)
						.getProfileName() != null
				&& !((PolicingProfilesEntriesResource) resource)
						.getProfileName().isEmpty()) {
			isValid = validator.isValidMaxLengthAlphaNum(
					((PolicingProfilesEntriesResource) resource)
							.getProfileName(), VtnServiceJsonConsts.LEN_32);
			setListOpFlag(true);
		} else if (resource instanceof PolicingProfilesEntryResource
				&& ((PolicingProfilesEntryResource) resource).getProfileName() != null
				&& !((PolicingProfilesEntryResource) resource).getProfileName()
						.isEmpty()) {
			isValid = validator
					.isValidMaxLengthAlphaNum(
							((PolicingProfilesEntryResource) resource)
									.getProfileName(),
							VtnServiceJsonConsts.LEN_32);
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.URI
						+ VtnServiceJsonConsts.SEQNUM);
				if (((PolicingProfilesEntryResource) resource).getSeqnum() != null
						&& !((PolicingProfilesEntryResource) resource)
								.getSeqnum().isEmpty()) {
					isValid = validator.isValidRange(
							((PolicingProfilesEntryResource) resource)
									.getSeqnum(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_255);
				} else {
					isValid = false;
				}
			}
			setListOpFlag(false);
		}
		LOG.trace("Complete PolicingProfilesEntriesValidation#validateUri()");
		return isValid;
	}

	/**
	 * validate request Json object for get post and put methods of
	 * PolicingProfilesEntriesValidation
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start PolicingProfilesEntriesValidation#validate()");
		LOG.info("Validating request for " + method
				+ " of PolicingProfilesEntriesValidation");
		boolean isValid = false;
		try {
			isValid = validateUri();
			if (isValid && requestBody != null
					&& VtnServiceConsts.GET.equals(method)) {
				isValid = validator.isValidGetForIntIndex(requestBody,
						isListOpFlag());
				setInvalidParameter(validator.getInvalidParameter());
				updateOpParameterForList(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.PUT.equals(method)) {
				isValid = validatePut(requestBody);
			} else if (isValid && requestBody != null
					&& VtnServiceConsts.POST.equals(method)) {
				isValid = validatePost(requestBody);
			} else if (isValid) {
				setInvalidParameter(VtnServiceConsts.INCORRECT_METHOD_INVOCATION);
				isValid = false;
			}
		} catch (final NumberFormatException e) {
			if (method.equals(VtnServiceConsts.POST)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:NumberFormatException");
			isValid = false;
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.POST)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:ClassCastException");
			isValid = false;
		}
		// Throws exception if validation fails
		if (!isValid) {
			LOG.error("Validation failed");
			throw new VtnServiceException(Thread.currentThread()
					.getStackTrace()[1].getMethodName(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorCode(),
					UncJavaAPIErrorCode.VALIDATION_ERROR.getErrorMessage());
		}
		LOG.info("Validation successful");
		LOG.trace("Complete PolicingProfilesEntriesResourceValidation#validate()");
	}

	/**
	 * /* validate put request Json object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePut(JsonObject requestBody) {
		LOG.trace("Start PolicingProfilesEntriesValidation#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.POLCINGPROFILEENTRY);
		if (requestBody.has(VtnServiceJsonConsts.POLCINGPROFILEENTRY)
				&& requestBody.get(VtnServiceJsonConsts.POLCINGPROFILEENTRY)
						.isJsonObject()) {
			final JsonObject profileEntryJson = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLCINGPROFILEENTRY);
			// validation for key: fl_name
			setInvalidParameter(VtnServiceJsonConsts.FLNAME);
			if (profileEntryJson.has(VtnServiceJsonConsts.FLNAME)
					&& profileEntryJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.FLNAME).getAsString() != null
					&& !profileEntryJson.get(VtnServiceJsonConsts.FLNAME)
							.getAsString().isEmpty()) {
				isValid = validator.isValidMaxLengthAlphaNum(profileEntryJson
						.getAsJsonPrimitive(VtnServiceJsonConsts.FLNAME)
						.getAsString(), VtnServiceJsonConsts.LEN_32);
			}
			// validation for tworatethreecolor (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.TWORATETHREECOLOR);
				if (profileEntryJson
						.has(VtnServiceJsonConsts.TWORATETHREECOLOR)) {
					final JsonObject twoRateThreeColorJson = profileEntryJson
							.getAsJsonObject(VtnServiceJsonConsts.TWORATETHREECOLOR);
					// validation for meter (optional)
					if (isValid) {
						isValid = validateMeter(twoRateThreeColorJson);
					}
					// validation for greenaction (optional)
					if (isValid) {
						isValid = validateGreenAction(twoRateThreeColorJson);
					}
					// validation for yellowaction (optional)
					if (isValid) {
						isValid = validateYellowAction(twoRateThreeColorJson);
					}
					// validation for redaction (optional)
					if (isValid) {
						isValid = validateRedAction(twoRateThreeColorJson);
					}
				}
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesValidation#validatePut()");
		return isValid;
	}

	/**
	 * /* validate post request Json object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start PolicingProfilesEntriesValidation#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.POLCINGPROFILEENTRY);
		if (requestBody.has(VtnServiceJsonConsts.POLCINGPROFILEENTRY)
				&& requestBody.get(VtnServiceJsonConsts.POLCINGPROFILEENTRY)
						.isJsonObject()) {
			final JsonObject profileEntryJson = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLCINGPROFILEENTRY);

			// validation for key: seqnum(mandatory)
			setInvalidParameter(VtnServiceJsonConsts.SEQNUM);
			if (profileEntryJson.has(VtnServiceJsonConsts.SEQNUM)
					&& profileEntryJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.SEQNUM).getAsString() != null) {
				isValid = validator.isValidRange(profileEntryJson
						.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
						.getAsString(), VtnServiceJsonConsts.VAL_1,
						VtnServiceJsonConsts.VAL_65535);
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesValidation#validatePost()");
		return isValid;
	}

	/**
	 * /* validate post redAction Json object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateRedAction(final JsonObject twoRateThreeColorJson) {
		LOG.trace("Start PolicingProfilesEntriesValidation#validatePost#validateRedAction()");
		setInvalidParameter(VtnServiceJsonConsts.REDACTION);
		boolean isValid = true;
		if (twoRateThreeColorJson.has(VtnServiceJsonConsts.REDACTION)) {
			final JsonObject redActionJson = twoRateThreeColorJson
					.getAsJsonObject(VtnServiceJsonConsts.REDACTION);

			// validation for yellow_action (optional)
			setInvalidParameter(VtnServiceJsonConsts.RED_ACTION);
			if (redActionJson.has(VtnServiceJsonConsts.RED_ACTION)
					&& redActionJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.RED_ACTION).getAsString() != null) {
				String rate = redActionJson.getAsJsonPrimitive(
						VtnServiceJsonConsts.RED_ACTION).getAsString();
				isValid = rate.equalsIgnoreCase(VtnServiceJsonConsts.PASS)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.DROP)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.PENALTY);

			}
			// validation for ga_priority (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.RAPRIORITY);
				if (redActionJson.has(VtnServiceJsonConsts.RAPRIORITY)
						&& redActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.RAPRIORITY).getAsString() != null) {
					isValid = validator.isValidRange(
							redActionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.RAPRIORITY)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_7);

				}
			}
			// validation for ga_dscp (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.RADSCP);
				if (redActionJson.has(VtnServiceJsonConsts.RADSCP)
						&& redActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.RADSCP).getAsString() != null) {
					isValid = validator.isValidRange(redActionJson
							.getAsJsonPrimitive(VtnServiceJsonConsts.RADSCP)
							.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_63);

				}
			}
			// validation for ga_drop_precedence (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.RADROPPRECEDENCE);
				if (redActionJson.has(VtnServiceJsonConsts.RADROPPRECEDENCE)
						&& redActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.RADROPPRECEDENCE)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							redActionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.RADROPPRECEDENCE)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_3);

				}
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesValidation#validatePost#validateRedAction()");
		return isValid;
	}

	/**
	 * /* validate post yellowAction Json object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateYellowAction(final JsonObject twoRateThreeColorJson) {
		LOG.trace("Start PolicingProfilesEntriesValidation#validatePost#validateYellowAction()");
		setInvalidParameter(VtnServiceJsonConsts.YELLOWACTION);
		boolean isValid = true;
		if (twoRateThreeColorJson.has(VtnServiceJsonConsts.YELLOWACTION)) {
			final JsonObject yellowActionJson = twoRateThreeColorJson
					.getAsJsonObject(VtnServiceJsonConsts.YELLOWACTION);

			// validation for yellow_action (optional)
			setInvalidParameter(VtnServiceJsonConsts.YELLOW_ACTION);
			if (yellowActionJson.has(VtnServiceJsonConsts.YELLOW_ACTION)
					&& yellowActionJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.YELLOW_ACTION).getAsString() != null) {
				String rate = yellowActionJson.getAsJsonPrimitive(
						VtnServiceJsonConsts.YELLOW_ACTION).getAsString();
				isValid = rate.equalsIgnoreCase(VtnServiceJsonConsts.PASS)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.DROP)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.PENALTY);

			}
			// validation for ga_priority (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.YAPRIORITY);
				if (yellowActionJson.has(VtnServiceJsonConsts.YAPRIORITY)
						&& yellowActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.YAPRIORITY).getAsString() != null) {
					isValid = validator.isValidRange(
							yellowActionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.YAPRIORITY)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_7);

				}
			}
			// validation for ga_dscp (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.YADSCP);
				if (yellowActionJson.has(VtnServiceJsonConsts.YADSCP)
						&& yellowActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.YADSCP).getAsString() != null) {
					isValid = validator.isValidRange(yellowActionJson
							.getAsJsonPrimitive(VtnServiceJsonConsts.YADSCP)
							.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_63);

				}
			}
			// validation for ga_drop_precedence (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.YADROPPRECEDENCE);
				if (yellowActionJson.has(VtnServiceJsonConsts.YADROPPRECEDENCE)
						&& yellowActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.YADROPPRECEDENCE)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							yellowActionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.YADROPPRECEDENCE)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_3);

				}
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesValidation#validatePost#validateYellowAction()");
		return isValid;
	}

	/**
	 * /* validate post greenAction Json object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateGreenAction(final JsonObject twoRateThreeColorJson) {
		LOG.trace("Start PolicingProfilesEntriesValidation#validatePost#validateGreenAction()");
		setInvalidParameter(VtnServiceJsonConsts.GREEN_ACTION);
		boolean isValid = true;
		if (twoRateThreeColorJson.has(VtnServiceJsonConsts.GREEN_ACTION)) {
			final JsonObject greenActionJson = twoRateThreeColorJson
					.getAsJsonObject(VtnServiceJsonConsts.GREEN_ACTION);

			// validation for green_action (optional)
			setInvalidParameter(VtnServiceJsonConsts.GREENACTION);
			if (greenActionJson.has(VtnServiceJsonConsts.GREENACTION)
					&& greenActionJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.GREENACTION).getAsString() != null) {
				String rate = greenActionJson.getAsJsonPrimitive(
						VtnServiceJsonConsts.GREENACTION).getAsString();
				isValid = rate.equalsIgnoreCase(VtnServiceJsonConsts.PASS)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.DROP)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.PENALTY);

			}
			// validation for ga_priority (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.GAPRIORITY);
				if (greenActionJson.has(VtnServiceJsonConsts.GAPRIORITY)
						&& greenActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.GAPRIORITY).getAsString() != null) {
					isValid = validator.isValidRange(
							greenActionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.GAPRIORITY)
									.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_7);

				}
			}
			// validation for ga_dscp (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.GADSCP);
				if (greenActionJson.has(VtnServiceJsonConsts.GADSCP)
						&& greenActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.GADSCP).getAsString() != null) {
					isValid = validator.isValidRange(greenActionJson
							.getAsJsonPrimitive(VtnServiceJsonConsts.GADSCP)
							.getAsString(), VtnServiceJsonConsts.VAL_0,
							VtnServiceJsonConsts.VAL_63);

				}
			}
			// validation for ga_drop_precedence (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.GADROPPRECEDENCE);
				if (greenActionJson.has(VtnServiceJsonConsts.GADROPPRECEDENCE)
						&& greenActionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.GADROPPRECEDENCE)
								.getAsString() != null) {
					isValid = validator.isValidRange(
							greenActionJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.GADROPPRECEDENCE)
									.getAsString(), VtnServiceJsonConsts.VAL_1,
							VtnServiceJsonConsts.VAL_3);

				}
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesValidation#validatePost#validateGreenAction()");
		return isValid;

	}

	/**
	 * validate post meterAction Json object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request Json object
	 * @return true, if successful
	 */
	private boolean validateMeter(final JsonObject twoRateThreeColorJson) {
		LOG.trace("Start PolicingProfilesEntriesValidation#validatePost#validateMeter()");
		setInvalidParameter(VtnServiceJsonConsts.METER);
		boolean isValid = true;
		if (twoRateThreeColorJson.has(VtnServiceJsonConsts.METER)) {
			final JsonObject meterJson = twoRateThreeColorJson
					.getAsJsonObject(VtnServiceJsonConsts.METER);

			// validation for rateunit (optional)
			setInvalidParameter(VtnServiceJsonConsts.RATEUNIT);
			if (meterJson.has(VtnServiceJsonConsts.RATEUNIT)
					&& meterJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.RATEUNIT).getAsString() != null) {
				String rate = meterJson.getAsJsonPrimitive(
						VtnServiceJsonConsts.RATEUNIT).getAsString();
				isValid = rate.equalsIgnoreCase(VtnServiceJsonConsts.KBPS)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.PPS);
			}
			// validation for cbs (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CBS);
				if (meterJson.has(VtnServiceJsonConsts.CBS)
						&& meterJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.CBS).getAsString() != null) {
					isValid = validator.isValidBigIntegerRangeString(
							new BigInteger(meterJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.CBS).getAsString()),
							VtnServiceJsonConsts.BIG_VAL0,
							VtnServiceJsonConsts.BIG_VAL_4294967295);

				}
			}
			// validation for pir (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.PIR);
				if (meterJson.has(VtnServiceJsonConsts.PIR)
						&& meterJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.PIR).getAsString() != null) {
					isValid = validator.isValidBigIntegerRangeString(
							new BigInteger(meterJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.PIR).getAsString()),
							VtnServiceJsonConsts.BIG_VAL0,
							VtnServiceJsonConsts.BIG_VAL_4294967295);

				}
			}
			// validation for pbs (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.PBS);
				if (meterJson.has(VtnServiceJsonConsts.PBS)
						&& meterJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.PBS).getAsString() != null) {
					isValid = validator.isValidBigIntegerRangeString(
							new BigInteger(meterJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.PBS).getAsString()),
							VtnServiceJsonConsts.BIG_VAL0,
							VtnServiceJsonConsts.BIG_VAL_4294967295);

				}
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesValidation#validatePost#validateMeter()");
		return isValid;
	}
}
