;;; .dir-locals.el --

;;; Commentary:

;;; Code:
(
 (c-mode .
         ((eval  . (progn

                     (let ((includes-list (list
                                           (substitute-in-file-name "$rcsw/include")
                                           (substitute-in-file-name "$rcsw/ext")
                                           (concat (projectile-project-root) "include")
                                           )))
                       (setq flycheck-clang-include-path includes-list)
                       (setq flycheck-gcc-include-path includes-list)
                       (setq flycheck-clang-args '("-std=gnu11"))
                       (add-to-list 'flycheck-clang-definitions
                                    "LIBRA_ER=LIBRA_ERL_ALL")

                       (setq flycheck-gcc-args '("-std=gnu11"))
                       (add-to-list 'flycheck-gcc-definitions "LIBRA_ERL=LIBRA_ERL_ALL")

                       (setq cc-search-directories includes-list)
                       )
                     )
                 ))
         )
 (nil . ((eval .
               (progn
                 (add-to-list 'projectile-globally-ignored-directories
                              "docs/_build")
                 (add-to-list 'projectile-globally-ignored-directories
                              "docs/_api")
                 )
               ))
      )
)
;;; end of .dir-locals.el
