/*
 * gnome-keyring
 *
 * Copyright (C) 2011 Collabora Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Author: Stef Walter <stefw@collabora.co.uk>
 */

#include "config.h"

#include "gcr-fingerprint.h"
#include "gcr-oids.h"
#include "gcr-subject-public-key.h"

#include "egg/egg-asn1x.h"
#include "egg/egg-asn1-defs.h"

#include <glib.h>

/**
 * SECTION:gcr-fingerprint
 * @title: Key Fingerprints
 * @short_description: Fingerprints for public and private keys
 *
 * These functions generate key fingerprints for public keys, certificates and
 * key data. The fingerprints are created so that they they will be identical
 * for a key and its corresponding certificate.
 *
 * Note that in the case of certificates these are not fingerprints of the
 * actual certificate data, but rather of the public key contained in a
 * certificate.
 *
 * These fingerprints are created using the subjectPublicKeyInfo ASN.1 structure.
 */

/**
 * gcr_fingerprint_from_subject_public_key_info:
 * @key_info: (array length=n_key_info): DER encoded subjectPublicKeyInfo structure
 * @n_key_info: length of DER encoded structure
 * @checksum_type: the type of fingerprint to create
 * @n_fingerprint: the length of fingerprint returned
 *
 * Create a key fingerprint for a DER encoded subjectPublicKeyInfo.
 *
 * Returns: (transfer full) (allow-none) (array length=n_fingerprint): the
 *          fingerprint or %NULL if the input was invalid.
 */
guchar *
gcr_fingerprint_from_subject_public_key_info (const guchar *key_info,
                                              gsize n_key_info,
                                              GChecksumType checksum_type,
                                              gsize *n_fingerprint)
{
	GChecksum *check;
	guint8 *fingerprint;

	g_return_val_if_fail (key_info, NULL);
	g_return_val_if_fail (n_key_info, NULL);
	g_return_val_if_fail (n_fingerprint, NULL);

	check = g_checksum_new (checksum_type);
	g_return_val_if_fail (check, NULL);

	g_checksum_update (check, key_info, n_key_info);

	*n_fingerprint = g_checksum_type_get_length (checksum_type);
	fingerprint = g_malloc (*n_fingerprint);
	g_checksum_get_digest (check, fingerprint, n_fingerprint);

	g_checksum_free (check);
	return fingerprint;
}

/**
 * gcr_fingerprint_from_attributes:
 * @attrs: attributes for key or certificate
 * @checksum_type: the type of fingerprint to create
 * @n_fingerprint: the length of fingerprint returned
 *
 * Create a key fingerprint for a certificate, public key or private key.
 * Note that this is not a fingerprint of certificate data, which you would
 * use gcr_certificate_get_fingerprint() for.
 *
 * Returns: (transfer full) (allow-none) (array length=n_fingerprint): the
 *          fingerprint or %NULL if the input was invalid.
 */
guchar *
gcr_fingerprint_from_attributes (GckAttributes *attrs,
                                 GChecksumType checksum_type,
                                 gsize *n_fingerprint)
{
	gpointer fingerprint = NULL;
	EggBytes *info;
	GNode *asn;

	g_return_val_if_fail (attrs != NULL, NULL);
	g_return_val_if_fail (n_fingerprint, NULL);

	asn = _gcr_subject_public_key_for_attributes (attrs);

	if (asn != NULL) {
		info = egg_asn1x_encode (asn, NULL);
		fingerprint = gcr_fingerprint_from_subject_public_key_info (egg_bytes_get_data (info),
		                                                            egg_bytes_get_size (info),
		                                                            checksum_type,
		                                                            n_fingerprint);
		egg_bytes_unref (info);
	}

	egg_asn1x_destroy (asn);
	return fingerprint;
}
