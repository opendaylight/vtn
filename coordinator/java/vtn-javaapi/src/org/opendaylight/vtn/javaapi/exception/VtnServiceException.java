/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.exception;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.javaapi.constants.VtnServiceConsts;

/**
 * The Class VtnServiceException.
 */
public class VtnServiceException extends Exception {

	/**
	 *
	 */
	private static final long serialVersionUID = -4276791776137640818L;

	protected transient List<SummaryItem> summaryItems = new ArrayList<SummaryItem>();

	/**
	 * The Class SummaryItem.
	 */
	protected class SummaryItem {
		public transient String errorContext;
		public transient String errorCode;
		public transient String errorText;

		/**
		 * Instantiates a new summary item.
		 * 
		 * @param contextCode
		 *            the context code
		 * @param errorCode
		 *            the error code
		 * @param errorText
		 *            the error text
		 */
		public SummaryItem(final String contextCode, final String errorCode,
				final String errorText) {

			this.errorContext = contextCode;
			this.errorCode = errorCode;
			this.errorText = errorText;
		}
	}

	/**
	 * Instantiates a new vtn service exception.
	 * 
	 * @param errorContext
	 *            the error context
	 * @param errorCode
	 *            the error code
	 * @param errorMessage
	 *            the error message
	 */
	public VtnServiceException(final String errorContext,
			final String errorCode, final String errorMessage) {
		super();
		addInfo(errorContext, errorCode, errorMessage);
	}

	/**
	 * Instantiates a new vtn service exception.
	 * 
	 * @param errorContext
	 *            the error context
	 * @param errorCode
	 *            the error code
	 * @param errorMessage
	 *            the error message
	 * @param cause
	 *            the cause
	 */
	public VtnServiceException(final String errorContext,
			final String errorCode, final String errorMessage,
			final Throwable cause) {
		super(cause);
		addInfo(errorContext, errorCode, errorMessage);
	}

	/**
	 * Adds the info.
	 * 
	 * @param errorContext
	 *            the error context
	 * @param errorCode
	 *            the error code
	 * @param errorText
	 *            the error text
	 * @return the vtn service exception
	 */
	public final VtnServiceException addInfo(final String errorContext,
			final String errorCode, final String errorText) {

		this.summaryItems.add(new SummaryItem(errorContext, errorCode,
				errorText));
		return this;
	}

	/**
	 * Append exception.
	 * 
	 * @param builder
	 *            the builder
	 * @param throwable
	 *            the throwable
	 */
	private void appendException(final StringBuilder builder,
			final Throwable throwable) {
		if (throwable == null) {
			return;
		}
		appendException(builder, throwable.getCause());
		builder.append(throwable.toString());
		builder.append(VtnServiceConsts.NEW_LINE_CHAR);
	}

	/**
	 * Gets the code.
	 * 
	 * @return the code
	 */
	public final String getCode() {
		final StringBuilder builder = new StringBuilder();

		builder.append(VtnServiceConsts.OPEN_SQR_BRACKET);
		final int length = this.summaryItems.size() - 1;
		for (int i = length; i >= 0; i--) {
			final SummaryItem info = this.summaryItems.get(i);
			builder.append(info.errorCode);
			if (i != 0) {
				builder.append(VtnServiceConsts.COLON);
			}
		}
		builder.append(VtnServiceConsts.CLOSE_SQR_BRACKET);

		return builder.toString();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Throwable#toString()
	 */
	@Override
	public final String toString() {
		final StringBuilder builder = new StringBuilder();

		builder.append(getCode());
		builder.append(VtnServiceConsts.NEW_LINE_CHAR);

		// append additional context information.
		for (int i = this.summaryItems.size() - 1; i >= 0; i--) {
			final SummaryItem info = this.summaryItems.get(i);
			builder.append(VtnServiceConsts.OPEN_SQR_BRACKET);
			builder.append(info.errorContext);
			builder.append(VtnServiceConsts.CLOSE_SQR_BRACKET);
			builder.append(VtnServiceConsts.OPEN_SQR_BRACKET);
			builder.append(info.errorCode);
			builder.append(VtnServiceConsts.COLON);
			builder.append(info.errorText);
			builder.append(VtnServiceConsts.CLOSE_SQR_BRACKET);
			if (i > 0) {
				builder.append(VtnServiceConsts.NEW_LINE_CHAR);
			}
		}

		// append root causes and text from this exception first.
		if (getMessage() != null) {
			builder.append(VtnServiceConsts.NEW_LINE_CHAR);
			if (getCause() == null) {
				builder.append(getMessage());
			} else if (!getMessage().equals(getCause().toString())) {
				builder.append(getMessage());
			}
		}
		appendException(builder, getCause());

		return builder.toString();
	}

}
