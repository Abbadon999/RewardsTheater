// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2023, Lev Leontev

#include "TwitchAuthDialog.h"

#include <obs-module.h>
#include <obs.h>

#include <format>
#include <iostream>
#include <obs.hpp>

#include "HttpClient.h"
#include "ui_TwitchAuthDialog.h"

TwitchAuthDialog::TwitchAuthDialog(QWidget* parent, TwitchAuth& twitchAuth)
    : QDialog(parent), twitchAuth(twitchAuth), ui(std::make_unique<Ui::TwitchAuthDialog>()),
      errorMessageBox(new ErrorMessageBox(this)) {
    ui->setupUi(this);

    connect(ui->authenticateInBrowserButton, &QPushButton::clicked, &twitchAuth, &TwitchAuth::authenticate);
    connect(
        ui->authenticateWithAccessTokenButton,
        &QPushButton::clicked,
        this,
        &TwitchAuthDialog::authenticateWithAccessToken
    );
    connect(errorMessageBox, &QMessageBox::finished, this, &TwitchAuthDialog::showOurselvesAfterAuthMessageBox);

    connect(&twitchAuth, &TwitchAuth::onAuthenticationSuccess, this, &TwitchAuthDialog::hide);
    connect(
        &twitchAuth, &TwitchAuth::onAuthenticationFailure, this, &TwitchAuthDialog::showAuthenticationFailureMessage
    );
    connect(
        &twitchAuth,
        &TwitchAuth::onAccessTokenAboutToExpire,
        this,
        &TwitchAuthDialog::showAccessTokenAboutToExpireMessage
    );
}

TwitchAuthDialog::~TwitchAuthDialog() = default;

void TwitchAuthDialog::authenticateWithAccessToken() {
    twitchAuth.authenticateWithToken(ui->accessTokenEdit->text().toStdString());
}

void TwitchAuthDialog::showAuthenticationFailureMessage(std::exception_ptr reason) {
    std::string message;
    try {
        std::rethrow_exception(reason);
    } catch (const TwitchAuth::UnauthenticatedException&) {
        message = obs_module_text("TwitchAuthenticationFailedInvalid");
    } catch (const HttpClient::NetworkException&) {
        message = obs_module_text("TwitchAuthenticationFailedNetwork");
    } catch (const TwitchAuth::EmptyAccessTokenException&) {
        message = obs_module_text("TwitchAuthenticationFailedNoAccessToken");
    } catch (const std::exception& otherException) {
        message = std::vformat(
            obs_module_text("TwitchAuthenticationFailedOther"), std::make_format_args(otherException.what())
        );
    }

    showAuthenticationMessage(message);
}

void TwitchAuthDialog::showAccessTokenAboutToExpireMessage(std::chrono::seconds expiresIn) {
    int expiresInHours = std::ceil(expiresIn.count() / 3600);
    std::string message =
        std::vformat(obs_module_text("TwitchTokenAboutToExpire"), std::make_format_args(expiresInHours));
    showAuthenticationMessage(message);
}

void TwitchAuthDialog::showAuthenticationMessage(const std::string& message) {
    if (isVisible()) {
        errorMessageBox->setStandardButtons(QMessageBox::Ok);
    } else {
        errorMessageBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        errorMessageBox->button(QMessageBox::Ok)->setText(obs_module_text("LogInAgain"));
    }
    errorMessageBox->show(message);
}

void TwitchAuthDialog::showOurselvesAfterAuthMessageBox() {
    if (isVisible()) {
        return;
    }
    if (errorMessageBox->standardButton(errorMessageBox->clickedButton()) == QMessageBox::Ok) {
        show();
    }
}
