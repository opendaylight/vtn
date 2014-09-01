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
 * The Class PolicingProfilesEntriesResourceValidator validates request JSON
 * object for PolicingProfilesEntriesResource and PolicingProfilesEntryResource
 * API.
 */
public class PolicingProfilesEntriesResourceValidator extends
		VtnServiceValidator {
	private static final Logger LOG = Logger
			.getLogger(PolicingProfilesEntriesResourceValidator.class.getName());
	private final AbstractResource resource;
	private final CommonValidator validator = new CommonValidator();

	/**
	 * Instantiates a new Policing Profiles Entries Resource validationr.
	 * 
	 * @param resource
	 *            the instance of AbstractResource
	 */
	public PolicingProfilesEntriesResourceValidator(
			final AbstractResource resource) {
		this.resource = resource;
	}

	/**
	 * Validate URI parameters for PolicingProfilesEntriesResource
	 * 
	 * @return true, if successful
	 */
	@Override
	public final boolean validateUri() {
		LOG.trace("Start PolicingProfilesEntriesResourceValidator#validateUri()");
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
		LOG.trace("Complete PolicingProfilesEntriesResourceValidator#validateUri()");
		return isValid;
	}

	/**
	 * validate request JSON object for get post and put methods of
	 * PolicingProfilesEntriesValidation
	 */
	@Override
	public final void validate(final String method, final JsonObject requestBody)
			throws VtnServiceException {
		LOG.trace("Start PolicingProfilesEntriesResourceValidator#validate()");
		LOG.debug("Validating request for " + method
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
			if (method.equals(VtnServiceConsts.GET)) {
				setInvalidParameter(validator.getInvalidParameter());
			}
			LOG.error(e, "Inside catch:NumberFormatException");
			isValid = false;
		} catch (final ClassCastException e) {
			if (method.equals(VtnServiceConsts.GET)) {
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
		LOG.trace("Complete PolicingProfilesEntriesResourceValidator#validate()");
	}

	/**
	 * /* validate put request JSON object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validatePut(JsonObject requestBody) {
		LOG.trace("Start PolicingProfilesEntriesResourceValidator#validatePut()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.POLICINGPROFILEENTRY);
		if (requestBody.has(VtnServiceJsonConsts.POLICINGPROFILEENTRY)
				&& requestBody.get(VtnServiceJsonConsts.POLICINGPROFILEENTRY)
						.isJsonObject()) {
			final JsonObject profileEntryJson = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGPROFILEENTRY);
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
					// validation for green-action (optional)
					if (isValid) {
						setInvalidParameter(VtnServiceJsonConsts.GREENACTION);
						if (twoRateThreeColorJson
								.has(VtnServiceJsonConsts.GREENACTION)) {
							isValid = validateActionJson(twoRateThreeColorJson
									.get(VtnServiceJsonConsts.GREENACTION)
									.getAsJsonObject());
						} else {
							isValid = false;
						}
					}
					// validation for yellow-action (optional)
					if (isValid) {
						setInvalidParameter(VtnServiceJsonConsts.YELLOWACTION);
						if (twoRateThreeColorJson
								.has(VtnServiceJsonConsts.YELLOWACTION)) {
							isValid = validateActionJson(twoRateThreeColorJson
									.get(VtnServiceJsonConsts.YELLOWACTION)
									.getAsJsonObject());
						} else {
							isValid = false;
						}
					}
					// validation for redaction (optional)
					if (isValid) {
						setInvalidParameter(VtnServiceJsonConsts.REDACTION);
						if (twoRateThreeColorJson
								.has(VtnServiceJsonConsts.REDACTION)) {
							isValid = validateActionJson(twoRateThreeColorJson
									.get(VtnServiceJsonConsts.REDACTION)
									.getAsJsonObject());
						} else {
							isValid = false;
						}
					}
				}
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesResourceValidator#validatePut()");
		return isValid;
	}

	/**
	 * /* validate post request JSON object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validatePost(final JsonObject requestBody) {
		LOG.trace("Start PolicingProfilesEntriesResourceValidator#validatePost()");
		boolean isValid = false;
		setInvalidParameter(VtnServiceJsonConsts.POLICINGPROFILEENTRY);
		if (requestBody.has(VtnServiceJsonConsts.POLICINGPROFILEENTRY)
				&& requestBody.get(VtnServiceJsonConsts.POLICINGPROFILEENTRY)
						.isJsonObject()) {
			final JsonObject profileEntryJson = requestBody
					.getAsJsonObject(VtnServiceJsonConsts.POLICINGPROFILEENTRY);

			// validation for key: seqnum (mandatory)
			setInvalidParameter(VtnServiceJsonConsts.SEQNUM);
			if (profileEntryJson.has(VtnServiceJsonConsts.SEQNUM)
					&& profileEntryJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.SEQNUM).getAsString() != null) {
				isValid = validator.isValidRange(profileEntryJson
						.getAsJsonPrimitive(VtnServiceJsonConsts.SEQNUM)
						.getAsString(), VtnServiceJsonConsts.VAL_1,
						VtnServiceJsonConsts.VAL_255);
			}
			if (isValid) {
				isValid = validatePut(requestBody);
			}
		}
		LOG.trace("Complete PolicingProfilesEntriesResourceValidator#validatePost()");
		return isValid;
	}

	/**
	 * /* validate post greenAction JSON object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validateActionJson(final JsonObject actionJson) {
		LOG.trace("Start PolicingProfilesEntriesResourceValidator#validatePost#validateActionJson()");
		boolean isValid = true;
		// validation for green_action (optional)
		setInvalidParameter(VtnServiceJsonConsts.TYPE);
		if (actionJson.has(VtnServiceJsonConsts.TYPE)
				&& actionJson.getAsJsonPrimitive(VtnServiceJsonConsts.TYPE)
						.getAsString() != null) {
			String rate = actionJson.getAsJsonPrimitive(
					VtnServiceJsonConsts.TYPE).getAsString();
			isValid = rate.equalsIgnoreCase(VtnServiceJsonConsts.PASS)
					|| rate.equalsIgnoreCase(VtnServiceJsonConsts.DROP)
					|| rate.equalsIgnoreCase(VtnServiceJsonConsts.PENALTY);

		}
		// validation for ga_priority (optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.PRIORITY);
			if (actionJson.has(VtnServiceJsonConsts.PRIORITY)
					&& actionJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.PRIORITY).getAsString() != null) {
				isValid = validator.isValidRange(
						actionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.PRIORITY).getAsString(),
						VtnServiceJsonConsts.VAL_0, VtnServiceJsonConsts.VAL_7);

			}
		}
		// validation for ga_dscp (optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.DSCP);
			if (actionJson.has(VtnServiceJsonConsts.DSCP)
					&& actionJson.getAsJsonPrimitive(VtnServiceJsonConsts.DSCP)
							.getAsString() != null) {
				isValid = validator.isValidRange(
						actionJson
								.getAsJsonPrimitive(VtnServiceJsonConsts.DSCP)
								.getAsString(), VtnServiceJsonConsts.VAL_0,
						VtnServiceJsonConsts.VAL_63);

			}
		}
		// validation for ga_drop_precedence (optional)
		if (isValid) {
			setInvalidParameter(VtnServiceJsonConsts.DROPPRECEDENCE);
			if (actionJson.has(VtnServiceJsonConsts.DROPPRECEDENCE)
					&& actionJson.getAsJsonPrimitive(
							VtnServiceJsonConsts.DROPPRECEDENCE).getAsString() != null) {
				isValid = validator.isValidRange(
						actionJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.DROPPRECEDENCE)
								.getAsString(), VtnServiceJsonConsts.VAL_1,
						VtnServiceJsonConsts.VAL_3);

			}
		}
		LOG.trace("Complete PolicingProfilesEntriesResourceValidator#validatePost#validateGreenAction()");
		return isValid;

	}

	/**
	 * validate post meterAction JSON object for PolicingProfilesEntries
	 * 
	 * @param requestBody
	 *            the request JSON object
	 * @return true, if successful
	 */
	private boolean validateMeter(final JsonObject twoRateThreeColorJson) {
		LOG.trace("Start PolicingProfilesEntriesResourceValidator#validatePost#validateMeter()");
		setInvalidParameter(VtnServiceJsonConsts.METER);
		boolean isValid = false;
		if (twoRateThreeColorJson.has(VtnServiceJsonConsts.METER)) {
			isValid = true;
			final JsonObject meterJson = twoRateThreeColorJson
					.getAsJsonObject(VtnServiceJsonConsts.METER);

			// validation for rateunit (optional)
			setInvalidParameter(VtnServiceJsonConsts.UNIT);
			if (meterJson.has(VtnServiceJsonConsts.UNIT)
					&& meterJson.getAsJsonPrimitive(VtnServiceJsonConsts.UNIT)
							.getAsString() != null) {
				String rate = meterJson.getAsJsonPrimitive(
						VtnServiceJsonConsts.UNIT).getAsString();
				isValid = rate.equalsIgnoreCase(VtnServiceJsonConsts.KBPS)
						|| rate.equalsIgnoreCase(VtnServiceJsonConsts.PPS);
			}
			// validation for cir (optional)
			if (isValid) {
				setInvalidParameter(VtnServiceJsonConsts.CIR);
				if (meterJson.has(VtnServiceJsonConsts.CIR)
						&& meterJson.getAsJsonPrimitive(
								VtnServiceJsonConsts.CIR).getAsString() != null) {
					isValid = validator.isValidBigIntegerRangeString(
							new BigInteger(meterJson.getAsJsonPrimitive(
									VtnServiceJsonConsts.CIR).getAsString()),
							VtnServiceJsonConsts.BIG_VAL0,
							VtnServiceJsonConsts.BIG_VAL_4294967295);

				}
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
		LOG.trace("Complete PolicingProfilesEntriesResourceValidator#validatePost#validateMeter()");
		return isValid;
	}
}
